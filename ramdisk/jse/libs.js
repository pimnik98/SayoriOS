//<#JSE#
//#include console.js
//#include canvas.js
//#include system.js
//#JSE#>

var test = "hello";

let sumnet = function(x, y) { return x + y; };

let f =  function(a){
    console.log(a);
};

console.log(test);
console.log(1 + 1);
console.log(system.ticks());

var a = 0;

for (; a < 10; a++){
    f(sumnet(1,a));
}
