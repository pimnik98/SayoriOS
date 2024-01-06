console_log(canvas_fillStyle());
canvas_fillStyle("0xFFFFFF");
console_log(canvas_fillStyle());

canvas_lineWidth(4);

canvas_fillRect(25, 25, 100, 100);

canvas_clearRect(45, 45, 60, 60);

canvas_strokeRect(50, 50, 50, 50);

canvas_setPixel(20,20,"0xFF0000");