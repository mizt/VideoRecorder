#import <Cocoa/Cocoa.h>
#import "Recorder.h"

class PNGRecorder : public Recorder {
        
    private:
        
        int width;
        int height;
        int fps;
        
        int frameCount = 0;
                        
        bool isRunning = false;
        bool isFinish = true;
        bool isWait = false;
        
        NSMutableString *path = [NSMutableString stringWithString:@""];
              
    public:
                
        PNGRecorder() {}
        
        void start(int width, int height, int fps=30, bool isSandobox=false) {
            
            if(!this->isRunning&&this->isFinish&&!this->isWait) {
            
                this->width  = width;
                this->height = height;
                this->fps = fps;
                                
                long unixtime = (CFAbsoluteTimeGetCurrent()+kCFAbsoluteTimeIntervalSince1970)*1000;
                NSString *timeStampString = [NSString stringWithFormat:@"%f",(unixtime/1000.0)];
                NSDate *date = [NSDate dateWithTimeIntervalSince1970:[timeStampString doubleValue]];
                NSDateFormatter *format = [[NSDateFormatter alloc] init];
                [format setLocale:[[NSLocale alloc] initWithLocaleIdentifier:@"ja_JP"]];
                [format setDateFormat:@"yyyy_MM_dd_HH_mm_ss_SSS"];
                NSString *filename = [format stringFromDate:date];
                NSArray *paths = NSSearchPathForDirectoriesInDomains (NSMoviesDirectory,NSUserDomainMask, YES);
                                                            
                if(isSandobox) {
                    NSString *documentsDirectory = [paths objectAtIndex:0];
                    [path setString:[NSString stringWithFormat:@"%@/%@",documentsDirectory,filename]];
                }
                else {                 
                    [path setString:[NSString stringWithFormat:@"./%@",filename]];
                }
                
                
                BOOL isDirectory;
                NSFileManager *fileManager= [NSFileManager defaultManager];
                if(![fileManager fileExistsAtPath:path isDirectory:&isDirectory]) {
                    if(![fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:NULL]) {
                        NSLog(@"Error: Create folder failed %@", path);
                    }
                }
                
                NSLog(@"%@",path);
                
                this->frameCount = 0;
                this->isRunning  = true;            
            }
        }
        
        void add(unsigned int *data,int rows,bool isRGBA=false) {
            
              NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
                    initWithBitmapDataPlanes:NULL
                    pixelsWide:this->width
                    pixelsHigh:this->height
                    bitsPerSample:8
                    samplesPerPixel:4
                    hasAlpha:YES
                    isPlanar:NO
                    colorSpaceName:NSDeviceRGBColorSpace //NSCalibratedRGBColorSpace
                    bitmapFormat:NSBitmapFormatAlphaFirst
                    bytesPerRow:(width)<<2
                    bitsPerPixel:0
                ];
                                                
                unsigned int *bmp = (unsigned int *)[rep bitmapData];
             
            
            if(isRGBA) {
                    
                for(int i=0; i<this->height; i++) {
                    for(int j=0; j<this->width; j++) {
                        unsigned int argb = data[i*rows+j];
                        unsigned char r = (argb>>16)&0xFF;
                        unsigned char g = (argb>>8)&0xFF;
                        unsigned char b = (argb)&0xFF;
                        bmp[i*this->width+j] = r<<24|g<<16|b<<8|0xFF;
                    }
                }
            }
            else {
                    
                for(int i=0; i<this->height; i++) {
                    for(int j=0; j<this->width; j++) {
                        unsigned int argb = data[i*rows+j];
                        unsigned char r = (argb>>16)&0xFF;
                        unsigned char g = (argb>>8)&0xFF;
                        unsigned char b = (argb)&0xFF;
                        bmp[i*this->width+j] = b<<24|g<<16|r<<8|0xFF;
                    }
                }
            }
            
            [[rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}] writeToFile:[[NSString stringWithFormat:@"%@/%05d.png",path,this->frameCount] stringByExpandingTildeInPath] atomically:NO];
                           
            this->frameCount++;
            
        }
        
        void add(unsigned int *data,bool isRGBA=false) {
            this->add(data,this->width,isRGBA);
        }
        
        void stop() {
            if(this->isRunning&&!this->isWait) {
                this->isRunning = false;
                this->isFinish = true;
                this->isWait = false;
            }
        }
        
        ~PNGRecorder() {
            
        }
};