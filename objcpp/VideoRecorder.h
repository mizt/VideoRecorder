#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>

class VideoRecorder {
        
    private:
        
        int width;
        int height;
        int fps;
        
        AVAssetWriter *writer;
        AVAssetWriterInput *videoInput;
        AVAssetWriterInputPixelBufferAdaptor *adaptor;
        
        NSDictionary *settings;
        NSDictionary *attributes;
                        
        int frameCount = 0;
        
        unsigned int *raw;
        CVPixelBufferRef buffer = nullptr;
                
        bool isRunning = false;
        bool isFinish = true;
        bool isWait = false;
                
    public:
                
        VideoRecorder() {}
        
        void start(int width,int height,int fps=30) {
            if(!this->isRunning&&this->isFinish&&!this->isWait) {
                this->width  = width;
                this->height = height;
                this->fps = fps;                                
                this->raw = (unsigned int *)malloc(this->width*this->height*sizeof(unsigned int));
                CVPixelBufferCreateWithBytes(kCFAllocatorDefault,this->width,this->height,kCVPixelFormatType_32ARGB,this->raw,this->width*4,nil, nil, nil,&this->buffer);
                CVBufferSetAttachment(this->buffer,kCVImageBufferColorPrimariesKey,kCVImageBufferColorPrimaries_ITU_R_709_2,kCVAttachmentMode_ShouldPropagate);
                CVBufferSetAttachment(this->buffer,kCVImageBufferYCbCrMatrixKey,kCVImageBufferYCbCrMatrix_ITU_R_709_2,kCVAttachmentMode_ShouldPropagate);
                CVBufferSetAttachment(this->buffer,kCVImageBufferTransferFunctionKey,kCVImageBufferTransferFunction_ITU_R_709_2,kCVAttachmentMode_ShouldPropagate);                    
                long unixtime = (CFAbsoluteTimeGetCurrent()+kCFAbsoluteTimeIntervalSince1970)*1000;
                NSString *timeStampString = [NSString stringWithFormat:@"%f",(unixtime/1000.0)];
                NSDate *date = [NSDate dateWithTimeIntervalSince1970:[timeStampString doubleValue]];
                NSDateFormatter *format = [[NSDateFormatter alloc] init];
                [format setLocale:[[NSLocale alloc] initWithLocaleIdentifier:@"ja_JP"]];
                [format setDateFormat:@"yyyy_MM_dd_HH_mm_ss_SSS"];
                NSString *filename = [format stringFromDate:date];
                NSArray * paths = NSSearchPathForDirectoriesInDomains (NSMoviesDirectory,NSUserDomainMask, YES);
                NSString * documentsDirectory = [paths objectAtIndex:0];
                NSString *path = [NSString stringWithFormat:@"%@/%@.mov",@".",filename];                                
                this->writer = [[AVAssetWriter alloc] initWithURL:[NSURL fileURLWithPath:path] fileType:AVFileTypeQuickTimeMovie error:nil];            
                this->settings = [NSDictionary dictionaryWithObjectsAndKeys:AVVideoCodecTypeAppleProRes4444,AVVideoCodecKey,[NSNumber numberWithInt:this->width],AVVideoWidthKey,[NSNumber numberWithInt:this->height],AVVideoHeightKey,nil];
                this->videoInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:this->settings];
                this->attributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithInt:kCVPixelFormatType_32ARGB],kCVPixelBufferPixelFormatTypeKey,nil];
                this->adaptor = [AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:this->videoInput sourcePixelBufferAttributes:this->attributes];
                this->videoInput.expectsMediaDataInRealTime = YES;
                [this->writer addInput:this->videoInput];
                [this->writer startWriting];
                [this->writer startSessionAtSourceTime:kCMTimeZero];
                this->frameCount = 0;
                this->isRunning  = true;            
            }
        }
        
        void add(unsigned int *data,int rows) {
            if(this->isRunning&&this->isFinish&&!this->isWait) {
                while(true) {
                    if(adaptor.assetWriterInput.readyForMoreMediaData) {
                        for(int i=0; i<this->height; i++) {
                            for(int j=0; j<this->width; j++) {
                                unsigned int argb = data[i*rows+j];
                                unsigned char r = (argb>>16)&0xFF;
                                unsigned char g = (argb>>8)&0xFF;
                                unsigned char b = (argb)&0xFF;
                                this->raw[i*this->width+j] = b<<24|g<<16|r<<8|255; // bgra                           
                            }
                        }
                        CVPixelBufferLockBaseAddress(this->buffer,0);                        
                        [this->adaptor appendPixelBuffer:this->buffer withPresentationTime:CMTimeMake((this->frameCount++)*(600.0/(double)this->fps),600)];
                        CVPixelBufferUnlockBaseAddress(this->buffer,0);
                        break;
                    }
                    else {
                        [NSThread sleepForTimeInterval:(1.0/600.)];
                    }
                }
            }
        }
        
        void stop() {
            if(this->isRunning&&!this->isWait) {
                this->isRunning = false;
                this->isFinish = false; 
                this->isWait = true;
                [this->videoInput markAsFinished];
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW,NSEC_PER_SEC),dispatch_get_main_queue(),^{
                    dispatch_async(dispatch_get_main_queue(),^{
                        [this->writer finishWritingWithCompletionHandler:^{
                            CVPixelBufferPoolRelease(this->adaptor.pixelBufferPool);
                            CVBufferRelease(this->buffer);
                            free((void *)this->raw);
                            this->isFinish = true;
                            this->isWait = false;
                            [[NSNotificationCenter defaultCenter] postNotificationName:@"done" object:nil userInfo:nil];
                        }];
                    });
                });
            }
        }
        
        ~VideoRecorder() {}
};