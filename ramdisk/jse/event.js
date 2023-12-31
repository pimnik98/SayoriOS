/// Просто пример для работы с мышью
var old_x = 0;
var old_y = 0;
var old_c = 0;
while(1){
    var m_combo = mouse(0);
    var m_x = mouse(1);
    var m_y = mouse(2);
    var m_time = mouse(3);

    if (
        (m_combo !== old_c) ||
        (m_x !== old_x) ||
        (m_y !== old_y)
    ){
        alert("x: ",m_x, "| y: ",m_y," | c: ", m_combo," | t: ",m_time);
        old_c = m_combo;
        old_x = m_x;
        old_y = m_y;
    }
    nop();
}



/// Пример для ожидания любой клавиши указанного из списка
while(0){
    var key = kbd_waitForAnyKeyOnce(57,28);
    alert("Key pressed: ", key);
    if (key === 57){
        alert(" |-- SPACE presed!");
    } else if (key === 28){
        alert(" |-- ENTER presed");
    }
}

/// Просто пример для работы с клавиатурой
while(0){
    var k_code = kbd(0);
    var k_time = kbd(1);
    var k_combo = kbd(2);
    if (k_code !== 0){
        alert("Code: ",k_code);
        alert("LastTime: ",k_time);
        alert("Combo: ",k_combo);
    }
    nop();
}
