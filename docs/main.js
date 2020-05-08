let isWindow = (typeof window !== 'undefined') ? true:false

let InBack  = function(t) { return 3*t*t*t-2*t*t; }
let OutBack = function(t) { return 1.-InBack(1.-t); }

let canvas;

const WIDTH = 1280;
const HEIGHT = 720;

if(isWindow) {
    canvas = document.createElement("canvas");
    canvas.width  = WIDTH;
    canvas.height = HEIGHT;
    
    canvas.style.background = "#CCC";
    
}
else {
    const { createCanvas } = require("canvas")
    canvas = createCanvas(WIDTH,HEIGHT)
}

let context = canvas.getContext("2d");


let rec;
if(!isWindow) {
    rec = require('../node/build/Release/VideoRecorder.node');
    rec.start(__dirname,canvas.width,canvas.height,30);
}

let w = 256;
let d = 16;
let ox = (canvas.width -w)>>1;  
let oy = (canvas.height-w)>>1;  
let p = [[0,0],[0,0],[0,0],[0,0]];

let cnt = 0;
let totalFrames = 16*30;

let tid = setInterval(function() {
    let v = 0;
    let n = cnt%(d<<1);
    
    if(n==0) {
        for(let k=0; k<4*2; k++) {
            p[k>>1][k&1] = 50+((Math.random()*0x7FFFFF)>>0)%100;
        }
    }
    
    if(n>d) {
        v = InBack(((d<<1)-n)/d);
    }
    else v = OutBack(n/d);
    
    context.clearRect(0,0,canvas.width,canvas.height);               
    
    let gradient = context.createLinearGradient(0, 0, 0, HEIGHT);

    gradient.addColorStop(0.15,"rgba(0,0,0,0)");
    gradient.addColorStop(0.85,"rgba(63,127,256,1)");
    
    context.fillStyle = gradient;
     
    context.beginPath();
        context.moveTo(ox+0-v*p[0][0],oy+0-v*p[0][1]);
        context.lineTo(ox+w+v*p[1][0],oy+0-v*p[1][1]);
        context.lineTo(ox+w+v*p[2][0],oy+w+v*p[2][1]);
        context.lineTo(ox+0-v*p[3][0],oy+w+v*p[3][1]);
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