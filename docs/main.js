var isWindow = (typeof window !== 'undefined') ? true:false

var InBack  = function(t) { return 3*t*t*t-2*t*t; }
var OutBack = function(t) { return 1.-InBack(1.-t); }

var canvas = (isWindow)?document.createElement("canvas"):(new (require("canvas"))());

var context = canvas.getContext("2d");
canvas.width  = 512;
canvas.height = 512;

var rec;
if(!isWindow) {
    rec = require('../node/build/Release/VideoRecorder.node');
    rec.start(__dirname,canvas.width,canvas.height,30);
}

var w = 100;
var d = 16;
var o = (canvas.width-w)>>1;  
var p = [[0,0],[0,0],[0,0],[0,0]];

var cnt = 0;
var totalFrames = 16*30;

var tid = setInterval(function(){
    var v = 0;
    var n = cnt%(d<<1);
    
    if(n==0) {
        for(var k=0; k<4*2; k++) {
            p[k>>1][k&1] = 10+((Math.random()*0x7FFFFF)>>0)%90;
        }
    }
    
    if(n>d) {
        v = InBack(((d<<1)-n)/d);
    }
    else v = OutBack(n/d);
    
    context.clearRect(0,0,canvas.width,canvas.height);               
    context.fillStyle = "rgba(0,0,255,0.8)";
    context.beginPath();
        context.moveTo(o+0-v*p[0][0],o+0-v*p[0][1]);
        context.lineTo(o+w+v*p[1][0],o+0-v*p[1][1]);
        context.lineTo(o+w+v*p[2][0],o+w+v*p[2][1]);
        context.lineTo(o+0-v*p[3][0],o+w+v*p[3][1]);
    context.closePath();
    context.fill();
    
    if(!isWindow) {
        if(cnt<totalFrames) {
            rec.add(context.getImageData(0,0,canvas.width,canvas.height).data);
        }
        else if(cnt===totalFrames) {
            rec.stop();
        }
        else {
            if(rec.isFinish()==true) clearInterval(tid);
        }            
    }
    cnt++;    
},33);

if(isWindow) document.body.appendChild(canvas);