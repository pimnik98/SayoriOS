// тестируем if else
/// Из-за сломанных цифр, не пашет нормально переменные и циклы
let a = 5;
let b = 9;
let c = false;
let d = true;
if (a === 3) {
    console_log("[1] [IF] It's 7\n");
} else if (a === 5) {
    console_log("[1] [ELSEIF] Hello");
} else {
    console_log("[1] [ELSE] Hello");
}

if (c){
    console_log("[2] [IF] c is true");
} else {
    console_log("[2] [ELSE] c is false");
}

if (d){
    console_log("[3] [IF] d is true");
} else {
    console_log("[3] [ELSE] d is false");
}
