<?php
define("SUCCESS"            , "1");                   ///< Задача успешно выполнена
define("FILE_NO_SELECTED"   ,"-1");                   ///< Файл не выбран
define("FILE_NO_ACCESS"     ,"-2");                   ///< Нет доступа к файлу
define("FILE_NO_CREATE"     ,"-3");                   ///< Не удалось создать файл
define("ARG1_NO_SPECIFIED"  ,"-4");                   ///< Аргумент1 не указан
define("FILE_ERR_BYTE_ZERO" ,"-5");                   ///< Количество байт должно быть <= 0
define("FILE_ERR_CREATE"    ,"-6");                   ///< Ошибка при создании файла или папки
define("FILE_ERR_DELETE"    ,"-7");                   ///< Ошибка при удалении файла
define("FILE_ERR_404"       ,"-8");                   ///< Файл не найден
define("IS_NO_DIR"          ,"-9");                   ///< Элемент != папке
define("FILE_IS_DIR"        ,"-10");                  ///< Этот элемент является папкой
define("FILE_ERR_NO_READ"   ,"-11");                  ///< Этот элемент недоступен для чтения
define("FILE_ERR_NO_WRITE"  ,"-12");                  ///< Этот элемент недоступен для записи

$owner = get_current_user();

function getCountFile($path){
    $dir = opendir($path);
    $count = 0;
    while($file = readdir($dir)){
        if($file == '.' || $file == '..'){
            continue;
        }
        $count++;
    }
    return $count;
}

function SayoriClean(){
    global $socket,$_tmp_file,$_act,$_bytes;
    $_tmp_file = "";
    $_bytes = 0;
    $_act = 0;
}

function SayoriGet(){
    global $socket;
    $data = "";
    $timer = time();
    $old = "";
    while(1){
        $buf = socket_read($socket, 1,PHP_BINARY_READ);
        if ($buf == "") continue;
        $data .= $buf;
        $last = (mb_substr($data, -6));
        if ($last == '|$MC#|'){
          $data = mb_substr($data,0,mb_strlen($data)-6);
          return [
              "data"  =>  $data,
              "size"  =>  mb_strlen($data)
          ];
        }
        //var_dump([$old,$buf,$i,$data]);
    }
}


function SayoriWriteFile(){
    global $socket,$_tmp_file,$_act,$_bytes;
    echo "SWF".PHP_EOL;
    var_dump($_tmp_file,$_act,$_bytes);
    $inx = 1;
    $data = "";
    while(1){
        if ($inx > $_bytes) break;
        $buf = socket_read($socket, 1,PHP_BINARY_READ);
        $data .= $buf;
        $inx++;
        //var_dump([$old,$buf,$i,$data]);
    }
    if (is_writable($_tmp_file)) {
        if (!$fp = fopen($_tmp_file, 'w+')) {
            echo "Cannot open file ($_tmp_file)".PHP_EOL;
            return -2;
        }
        if (fwrite($fp, $data) === FALSE) {
            echo "Cannot write to file ($_tmp_file)".PHP_EOL;
            return -3;
        }
        echo "Success, wrote ($data) to file ($_tmp_file)".PHP_EOL;
        fclose($fp);
        return $_bytes;
    } else {
        echo "The file $_tmp_file is not writable".PHP_EOL;
        return -1;
    }
}

function SayoriSet($sms){
    global $socket;
    $send = socket_send($socket,$sms,strlen($sms),MSG_EOF);
    var_dump($send);
}

function SayoriHandler($cmd){
    global $socket,$_tmp_file,$_act,$_bytes,$owner;
    $root = __DIR__."/server/";
    echo "> ".$cmd.PHP_EOL;
    $data = $cmd;
    $cmd = explode(" ",$cmd);
    switch($cmd[0]){
        case "READY":{
            $sms = SUCCESS;
            SayoriClean();
            break;
        }
        case "CLEAN":{
            $sms = "CLEAN";
            SayoriClean();
            break;
        }
        case "MKDIR":{
            SayoriClean();
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                break;
            }
            if (!mkdir($root.$cmd[1], 0777, true)) {
                $sms = FILE_ERR_CREATE;   // Не удалось создать папку
                break;
            }
            $sms = SUCCESS;
            break;
        }
        case "TOUCH":{
            SayoriClean();
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                break;
            }
            if (!touch($root.$cmd[1])) {
                $sms = FILE_ERR_CREATE;   // Не удалось создать файл
                break;
            }
            $sms = SUCCESS;
            break;
        }
        case "DEL":{
            SayoriClean();
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                break;
            }
            if (!file_exists($root.$cmd[1])){
                $sms = FILE_ERR_404;  // Аргумент не указан
                break;
            }
            $del = (is_dir($root.$cmd[1])?rmdir($root.$cmd[1]):unlink($root.$cmd[1]));
            if (!$del) {
                $sms = FILE_ERR_DELETE;   // Не удалось удалить файл
                break;
            }
            if (file_exists($root.$cmd[1])){
                $sms = FILE_ERR_DELETE;   // Не удалось удалить файл
                break;
            }
            $sms = SUCCESS;
            break;
        }
        case "COPY":{
            break;
        }
        case "MOVE":{
            break;
        }
        case "LIST":{
            if ($_act == 1){
                $elem = "";
                $dir = opendir($_tmp_file);
                while($file = readdir($dir)){
                    if($file == '.' || $file == '..'){
                        continue;
                    }
                    $type = (is_dir($_tmp_file.$file)?'dir':'file');
                    $pi = pathinfo($_tmp_file.$file);
                    $elem .= $type."::".$file."::".(isset($pi['extension'])?$pi['extension']:"---")."::".date("F d Y H-i-s", filemtime($_tmp_file.$file))."::".filesize($_tmp_file.$file)."::".$owner."\n";
                }
                $sms = $elem;
                $_act = 0;
                break;
            }
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                SayoriClean();
                break;
            }
            if (!file_exists($root.$cmd[1])){
                $sms = FILE_ERR_404;  // Аргумент не указан
                SayoriClean();
                break;
            }
            if (!is_dir($root.$cmd[1])){
                $sms = IS_NO_DIR;  // Это не папка
                SayoriClean();
                break;
            }
            $_tmp_file = $root.$cmd[1];
            $elem = "";
                $dir = opendir($_tmp_file);
                while($file = readdir($dir)){
                    if($file == '.' || $file == '..'){
                        continue;
                    }
                    $type = (is_dir($_tmp_file.$file)?'dir':'file');
                    $pi = pathinfo($_tmp_file.$file);
                    $elem .= $type."::".$file."::".(isset($pi['extension'])?$pi['extension']:"---")."::".date("F d Y H-i-s", filemtime($_tmp_file.$file))."::".filesize($_tmp_file.$file)."::".$owner."\n";
                }
                $sms = strlen($elem);

            //$sms = getCountFile($root.$cmd[1]);
            $_act = 1;
            break;
            # Type::Name::Format::Date::Size::UID
            //echo "";
        }
        case "READ":{
            if ($cmd[1] == "GET"){
                // Юзер запрашивает файлик :(
                // Придеться его отдать...
              if (!file_exists($_tmp_file) || $_tmp_file == ""){
                  // Если он вдруг пропал
                  $sms = "";
                  SayoriClean();
                  break;
              }
              //$sms = filesize($_tmp_file);
              $sms = file_get_contents($_tmp_file);
              SayoriClean();
              break;
            }
            if (!isset($cmd[1]) || !file_exists($root.$cmd[1])){
                $sms = FILE_NO_SELECTED;
                SayoriClean();
                break;
            }
            if (is_dir($root.$cmd[1])){
                $sms = FILE_IS_DIR;
                SayoriClean();
                break;
            }
            if (!is_readable($root.$cmd[1])){
                $sms = FILE_ERR_NO_READ;
                SayoriClean();
                break;
            }
            $sms = filesize($root.$cmd[1]);
            $_tmp_file = $root.$cmd[1];
            break;
        }
        case "WRITE":{
            if ($_act == 0){
                if (!isset($cmd[1])){
                    $sms = FILE_NO_SELECTED;  // Файл не указан
                    SayoriClean();
                    break;
                } elseif (!file_exists($root.$cmd[1])){
                    // Файл не найден
                    // Попробуем его создать
                    if (!touch($root.$cmd[1])) {
                        $sms = FILE_NO_CREATE;
                        SayoriClean();
                        break;
                    }
                }
                if (!is_readable($root.$cmd[1])){
                    $sms = FILE_ERR_NO_WRITE;
                    SayoriClean();
                    break;
                }
                $_tmp_file = $root.$cmd[1];
                $sms = SUCCESS;
                $_act = 1;
                break;
            } elseif ($_act == 1){
                if (!isset($cmd[1])){
                    $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                    SayoriClean();
                    break;
                }
                $_bytes = intval($cmd[1]);
                if ($_bytes < 0){
                    $sms = FILE_ERR_BYTE_ZERO;
                    SayoriClean();
                    break;
                }
                $_act = 9999;
                $sms = SUCCESS;
                break;
            }
        }
        default:{
            $sms = "UNKNOWN";
            break;
        }
    }
    SayoriSet($sms."\n");
    echo "< ".$sms.PHP_EOL.PHP_EOL;
}
header('Content-Type: text/plain;');    //Мы будем выводить простой текст
set_time_limit(0);                      //Скрипт должен работать постоянно
ob_implicit_flush();                    //Все echo должны сразу же выводиться
$address = '0.0.0.0';                   //Адрес работы сервера
$port = 10000;                          //Порт работы сервера (лучше какой-нибудь редкоиспользуемый)
$_tmp_file = "";
$_act = 0;
$_bytes = 0;
if (($socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) < 0) {
    //AF_INET - семейство протоколов
    //SOCK_STREAM - тип сокета
    //SOL_TCP - протокол
    echo "Ошибка создания сокета";
    } else {
    echo "Сокет создан\n";
}
while(1){
    $result = @socket_connect(@$socket, @$address, @$port);
    if ($result === false) {
        echo "Ошибка при подключении к сокету\n";
        sleep(1);
    } else {
        echo "Подключение к сокету прошло успешно\n";
        break;
    }
}
$data = "";
$mode = PHP_NORMAL_READ;
while(1){
    if ($_act == 9999){
        $ans = SayoriWriteFile();
        SayoriSet($ans."\n");
        SayoriClean();
        $_act = 0;
    }
    $adata = SayoriGet();
    SayoriHandler($adata["data"]);
}
echo "Соединение завершено\n";
//Останавливаем работу с сокетом
if (isset($socket)) {
    socket_close($socket);
    echo "Сокет успешно закрыт";
}
?>
