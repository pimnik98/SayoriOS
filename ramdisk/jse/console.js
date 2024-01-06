/// Вообще надо писать console.log // console.warn И тд, но почему-то зарегистрировать такие нельзя.
let console = {
    log: function(a) {
        return console_log(a);
    },
    warn: function(a) {
        return console_warn(a);
    },
    debug: function(a) {
        return console_debug(a);
    },
    err: function(a) {
        return console_err(a);
    },
    note: function(a) {
        return console_note(a);
    }
};
//obj.f(3);
console.log("Hello World!");
console.warn("Hello NDRAEY!");
console.err("IS ERROR!");
console.note("Note!");
console.debug("SayoriOS Team","Presents");
print("Загляни в консоль :)");
