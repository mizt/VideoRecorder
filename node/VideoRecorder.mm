#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import <node.h>

namespace mizt {
    namespace VideoRecorder {
        
        int width = 512;
        int height = 512;
        int fps = 30;
        
        AVAssetWriter *writer;
        AVAssetWriterInput *videoInput;
        
        AVAssetWriterInputPixelBufferAdaptor *adaptor;
        
        NSDictionary *settings;
        NSDictionary *attributes;
                        
        int frameCount = 0;
        
        unsigned int *raw = nullptr;
        CVPixelBufferRef buffer = nullptr;
        
        bool isRunning = false;
        bool isFinish = false;
        bool isWait = false;
                
        void onStart(const v8::FunctionCallbackInfo<v8::Value>&args) {
            if(!isRunning&&!isFinish&&!isWait) {
                v8::HandleScope handle_scope(args.GetIsolate());
                if(args.Length()==4) {
                    if(args[1]->IsNumber()&&args[2]->IsNumber()&&args[3]->IsNumber()) {
                       v8::Isolate *isolate = args.GetIsolate();
                        v8::String::Utf8Value dirname(isolate, args[0]);
                        if(*dirname) {
                            v8::Local<v8::Context> context = isolate->GetCurrentContext();
                            width  = (int)args[1]->NumberValue(context).FromMaybe(512);
                            height = (int)args[2]->NumberValue(context).FromMaybe(512);
                            fps = (int)args[3]->NumberValue(context).FromMaybe(30);
                            long unixtime = (CFAbsoluteTimeGetCurrent()+kCFAbsoluteTimeIntervalSince1970)*1000;
                            NSString *timeStampString = [NSString stringWithFormat:@"%f",(unixtime/1000.0)];
                            NSDate *date = [NSDate dateWithTimeIntervalSince1970:[timeStampString doubleValue]];
                            NSDateFormatter *format = [[NSDateFormatter alloc] init];
                            [format setLocale:[[NSLocale alloc] initWithLocaleIdentifier:@"ja_JP"]];
                            [format setDateFormat:@"yyyy_MM_dd_HH_mm_ss_SSS"];                                
                            NSString *path = [NSString stringWithFormat:@"%s/%@.mov",(const char*)*dirname,[format stringFromDate:date]];
                            NSLog(@"start %@ %dx%d %dfps",path,width,height,fps);
                            if(raw) free((void *)raw);
                            raw = (unsigned int *)malloc(width*height*sizeof(unsigned int));
                            CVPixelBufferCreateWithBytes(kCFAllocatorDefault,width,height,kCVPixelFormatType_32ARGB,raw,width<<2,nil,nil,nil,&buffer);
                            CVBufferSetAttachment(buffer,kCVImageBufferColorPrimariesKey,kCVImageBufferColorPrimaries_ITU_R_709_2,kCVAttachmentMode_ShouldPropagate);
                            CVBufferSetAttachment(buffer,kCVImageBufferYCbCrMatrixKey,kCVImageBufferYCbCrMatrix_ITU_R_709_2,kCVAttachmentMode_ShouldPropagate);
                            CVBufferSetAttachment(buffer,kCVImageBufferTransferFunctionKey,kCVImageBufferTransferFunction_ITU_R_709_2,kCVAttachmentMode_ShouldPropagate);
                            writer = [[AVAssetWriter alloc] initWithURL:[NSURL fileURLWithPath:path] fileType:AVFileTypeQuickTimeMovie error:nil];
                            settings = [NSDictionary dictionaryWithObjectsAndKeys:AVVideoCodecTypeAppleProRes4444,AVVideoCodecKey,[NSNumber numberWithInt:width],AVVideoWidthKey,[NSNumber numberWithInt:height],AVVideoHeightKey,nil];
                            videoInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:settings];
                            attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32ARGB],kCVPixelBufferPixelFormatTypeKey,nil];
                            adaptor = [AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:videoInput sourcePixelBufferAttributes:attributes];
                            videoInput.expectsMediaDataInRealTime = YES;
                            [writer addInput:videoInput];
                            [writer startWriting];
                            [writer startSessionAtSourceTime:kCMTimeZero];
                            frameCount = 0;
                            isRunning = true;
                        }
                    }
                }      
            }     
        }
        
        void onAdd(const v8::FunctionCallbackInfo<v8::Value>&args) {
            NSLog(@"add");
            if(isRunning&&!isFinish&&!isWait) {
                v8::HandleScope handle_scope(args.GetIsolate());
                v8::Local<v8::Uint8ClampedArray> array = args[0].As<v8::Uint8ClampedArray>();
                unsigned int *src = (unsigned int*)array->Buffer()->GetContents().Data();
                while(1) {
                    if(adaptor.assetWriterInput.readyForMoreMediaData) {
                        for(int i=0; i<height; i++) {
                            for(int j=0; j<width; j++) {
                                int addr = i*width+j;
                                unsigned int abgr = src[addr];
                                raw[addr] = (abgr<<8)|(abgr>>24); // bgra                    
                            }
                        }
                        CVPixelBufferLockBaseAddress(buffer,0);                        
                        [adaptor appendPixelBuffer:buffer withPresentationTime:CMTimeMake((frameCount++)*(600.0/(double)fps),600)];
                        CVPixelBufferUnlockBaseAddress(buffer,0);
                        break;
                    }
                    else {
                        [NSThread sleepForTimeInterval:(1.0/600.)];
                    }
                }
            }
        }

        void onStop(const v8::FunctionCallbackInfo<v8::Value>&args) {
            NSLog(@"stop");
            if(isRunning&&!isWait&&!isFinish) {
                isRunning = false;
                isWait  = true;
                [videoInput markAsFinished];
                [writer finishWritingWithCompletionHandler:^{                                                    
                    CVPixelBufferPoolRelease(adaptor.pixelBufferPool);
                    CVBufferRelease(buffer);
                    if(raw) free((void *)raw);
                    isWait = false;
                    isFinish = true;  
                    NSLog(@"done");
                }];
            }
        }
        
        void onIsFinish(const v8::FunctionCallbackInfo<v8::Value>&args) {
            v8::Isolate* isolate = args.GetIsolate();
            args.GetReturnValue().Set(v8::Boolean::New(isolate,isFinish?true:false));
        }
        
        void init(v8::Local<v8::Object> exports) {
            NSLog(@"init");
            NODE_SET_METHOD(exports,"start",onStart);
            NODE_SET_METHOD(exports,"add",onAdd);
            NODE_SET_METHOD(exports,"stop",onStop);
            NODE_SET_METHOD(exports,"isFinish",onIsFinish);
        }

        NODE_MODULE(addon,init)
    }
}