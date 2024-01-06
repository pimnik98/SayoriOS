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
