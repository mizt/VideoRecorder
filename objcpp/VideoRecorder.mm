#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import "VideoRecorder.h"

class App {
    
    private:
         
        int width  = 512;
        int height = 512;
        int fps = 30;
            
        int cnt = 0;
        unsigned int *dst = nullptr;
        
        VideoRecorder *recorder = new VideoRecorder();
        
    public:
        
        App() {
            
            this->dst = new unsigned int[width*height];
                
            for(int i=0; i<height; i++) {
                for(int j=0; j<width; j++) {
                    dst[i*width+j] = 0xFF0000FF;
                }
            }

            [[NSNotificationCenter defaultCenter] addObserverForName:@"done" object:nil queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *notification) {
                NSLog(@"done");
                [NSApp terminate:nil];
            }];
            
            recorder->start(width,height,fps);
                
            dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,dispatch_queue_create("ENTER_FRAME",0));
            dispatch_source_set_timer(timer,dispatch_time(0,0),(1.0/(double)fps)*1000000000,0);
            dispatch_source_set_event_handler(timer,^{
                
                if(cnt>=100) {
                    recorder->stop();
                    if(timer) {
                        dispatch_source_cancel(timer);
                    }                                    
                }
                else {
                    recorder->add(dst,512);
                }
                
                cnt++;
                
            });
            if(timer) dispatch_resume(timer);
            
        }
        
        ~App() {
            delete[] dst;
        }
};

#pragma mark AppDelegate
@interface AppDelegate:NSObject <NSApplicationDelegate> {
    App *app;
}
@end
@implementation AppDelegate
-(void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    app = new App();
}
-(void)applicationWillTerminate:(NSNotification *)aNotification {
    delete app;
}
@end

int main(int argc, char *argv[]) {
    @autoreleasepool {
    
        id app = [NSApplication sharedApplication];
        id delegat = [AppDelegate alloc];
        [app setDelegate:delegat];
        [app run];
    }
}
