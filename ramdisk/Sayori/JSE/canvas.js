let canvas = {
    fillRect: function(x, y, width, height) {
        return canvas_fillRect(x, y, width, height);
    },
    rect: function(x, y, width, height) {
        return canvas_rect(x, y, width, height);
    },
    strokeRect: function(x, y, width, height) {
        return canvas_strokeRect(x, y, width, height);
    }
};
