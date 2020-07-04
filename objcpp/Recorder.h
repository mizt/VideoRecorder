class Recorder {
    
    public:
        
        Recorder() {            
        }
        
        ~Recorder() {
        }
        
        virtual void start(int width, int height, int fps=30, bool isSandobox=false) = 0;
        virtual void add(unsigned int *data,int rows,bool isRGBA=false) = 0;
        virtual void add(unsigned int *data,bool isRGBA=false) = 0;
        virtual void stop() = 0;
        
};