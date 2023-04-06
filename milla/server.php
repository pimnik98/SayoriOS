<?php
/**
 * ДАННЫЙ ФАЙЛ ДОЛЖЕН ОСТАВАТЬСЯ ТОЛЬКО В ПРИВАТНОМ РЕПОЗИТОРИИ!
 * НЕ РАСПРОСТРАНЯТЬ ЕГО!!!
 *
 * ЭТОТ ФАЙЛ ЧИСТО ДЛЯ ЛОКАЛЬНОЙ РАЗРАБОТКИ, ЕСЛИ НЕТ ВОЗМОЖНОСТИ ПОДКЛЮЧИТЬСЯ К ОБЛАКУ.
 */
$socket = stream_socket_server("tcp://0.0.0.0:10000", $errno, $errstr);

if (!$socket) {
    die("$errstr ($errno)\n");
}


define("NO_AUTH"            ,"-9999");                ///< Пользователь не авторизован

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


function SayoriClean($connect){
    global $_global;
    $_global[signChanell($connect)]["file"] = "";
    $_global[signChanell($connect)]["bytes"] = 0;
    $_global[signChanell($connect)]["act"] = 0;
}

function SayoriGet($connect){
    global $connects;
    $data = "";
    $timer = time();
    $old = "";
    while(1){
        $buf = fread($connect, 1);

        if (!$buf) { //соединение было закрыто
            //echo "NO DATA...".PHP_EOL;
            fclose($connect);
            unset($connects[ array_search($connect, $connects) ]);
            onClose($connect);//вызываем пользовательский сценарий
            return [
                "data"  =>  $data,
                "size"  =>  mb_strlen($data)
            ];
        }


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

function SayoriLogin($connect,$login,$pwd){
    global $_global;
    if ($login != "SayoriOS" || $pwd != "NatSuki"){
        SayoriSet(NO_AUTH."\n",$connect);
        return false;
    }
    $_global[signChanell($connect)]["auth"] = true;
    $_global[signChanell($connect)]["login"] = $login;
    $_global[signChanell($connect)]["pwd"] = $pwd;
    SayoriSet(SUCCESS."\n",$connect);
    return true;
}



function SayoriWriteFile($connect){
    global $_global;
    //echo "SWF".PHP_EOL;
    //var_dump($_global[signChanell($connect)]["file"],$_global[signChanell($connect)]["act"],$_global[signChanell($connect)]["bytes"]);
    $inx = 1;
    $data = "";
    while(1){
        if ($inx > $_global[signChanell($connect)]["bytes"]) break;
        $buf = fread($connect, 1);
        $data .= $buf;
        $inx++;
        //var_dump([$old,$buf,$i,$data]);
    }
    if (is_writable($_global[signChanell($connect)]["file"])) {
        if (!$fp = fopen($_global[signChanell($connect)]["file"], 'w+')) {
            //echo "Cannot open file (".$_global[signChanell($connect)]["file"].")".PHP_EOL;
            return -2;
        }
        if (fwrite($fp, $data) === FALSE) {
            //echo "Cannot write to file (".$_global[signChanell($connect)]["file"].")".PHP_EOL;
            return -3;
        }
        //echo "Success, wrote ($data) to file (".$_global[signChanell($connect)]["file"].")".PHP_EOL;
        fclose($fp);
        return $_global[signChanell($connect)]["bytes"];
    } else {
        //echo "The file ".$_global[signChanell($connect)]["file"]." is not writable".PHP_EOL;
        return -1;
    }
}

function SayoriSet($sms,$connect){
    $send = fwrite($connect,$sms,strlen($sms));
    //$send = socket_send($socket,$sms,strlen($sms),MSG_EOF);
    //var_dump($send);
}

function SayoriGetPath($path){
    echo $path." >> ";
    $path = str_replace('..',"",$path);
    $path = str_replace('\..\\',"",$path);
    $path = str_replace('\.\\',"",$path);
    $path = str_replace('\\',"",$path);
    $path = str_replace('//',"/",$path);
    $path = str_replace('../',"",$path);
    $path = str_replace('./',"",$path);
    echo $path.PHP_EOL;
    return $path;
}

function SayoriHandler($cmd,$connect){
    global $_global;
    $root = __DIR__."/server/";
    echo "> ".$cmd.PHP_EOL;
    $data = $cmd;
    $cmd = explode(" ",$cmd);

    if (!$_global[signChanell($connect)]["auth"]){
        if ($cmd[0] == "LOGIN"){
            $log = isset($cmd[1]) && !empty($cmd[1])?$cmd[1]:"";
            $pwd = isset($cmd[2]) && !empty($cmd[2])?$cmd[2]:"";
            $status = SayoriLogin($connect,$log,$pwd);
            echo "Connect ".(($status)?'successful':'error')." (".$_global[signChanell($connect)]["sign"].") ".$log.":".$pwd.PHP_EOL;
        }
        return 1;
    }

    switch($cmd[0]){
        case "READY":{
            $sms = SUCCESS;
            SayoriClean($connect);
            break;
        }
        case "CLEAN":{
            $sms = "CLEAN";
            SayoriClean($connect);
            break;
        }
        case "MKDIR":{
//             SayoriClean($connect);
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                break;
            }
            if (!mkdir($root.SayoriGetPath($cmd[1]), 0777, true)) {
                $sms = FILE_ERR_CREATE;   // Не удалось создать папку
                break;
            }
            $sms = SUCCESS;
            break;
        }
        case "TOUCH":{
            SayoriClean($connect);
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                break;
            }
            if (!touch($root.SayoriGetPath($cmd[1]))) {
                $sms = FILE_ERR_CREATE;   // Не удалось создать файл
                break;
            }
            $sms = SUCCESS;
            break;
        }
        case "DEL":{
            SayoriClean($connect);
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                break;
            }
            if (!file_exists($root.SayoriGetPath($cmd[1]))){
                $sms = FILE_ERR_404;  // Аргумент не указан
                break;
            }
            $del = (is_dir($root.SayoriGetPath($cmd[1]))?rmdir($root.SayoriGetPath($cmd[1])):unlink($root.SayoriGetPath($cmd[1])));
            if (!$del) {
                $sms = FILE_ERR_DELETE;   // Не удалось удалить файл
                break;
            }
            if (file_exists($root.SayoriGetPath($cmd[1]))){
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
            if ($_global[signChanell($connect)]["act"] == 1){
                $elem = "";
                $dir = opendir($_global[signChanell($connect)]["file"]);
                while($file = readdir($dir)){
                    if($file == '.' || $file == '..'){
                        continue;
                    }
                    $type = (is_dir($_global[signChanell($connect)]["file"].$file)?'dir':'file');
                    $pi = pathinfo($_global[signChanell($connect)]["file"].$file);
                    $elem .= $type."::".$file."::".(isset($pi['extension'])?$pi['extension']:"---")."::".date("F d Y H-i-s", filemtime($_global[signChanell($connect)]["file"].$file))."::".filesize($_global[signChanell($connect)]["file"].$file)."::".$_global[signChanell($connect)]["owner"]."\n";
                }
                $sms = $elem;
                $_global[signChanell($connect)]["act"] = 0;
                break;
            }
            if (!isset($cmd[1])){
                $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                SayoriClean($connect);
                break;
            }
            if (!file_exists($root.SayoriGetPath($cmd[1]))){
                $sms = FILE_ERR_404;  // Аргумент не указан
                SayoriClean($connect);
                break;
            }
            if (!is_dir($root.SayoriGetPath($cmd[1]))){
                $sms = IS_NO_DIR;  // Это не папка
                SayoriClean($connect);
                break;
            }
            $_global[signChanell($connect)]["file"] = $root.SayoriGetPath($cmd[1]);
            $elem = "";
                $dir = opendir($_global[signChanell($connect)]["file"]);
                while($file = readdir($dir)){
                    if($file == '.' || $file == '..'){
                        continue;
                    }
                    $type = (is_dir($_global[signChanell($connect)]["file"].$file)?'dir':'file');
                    $pi = pathinfo($_global[signChanell($connect)]["file"].$file);
                    $elem .= $type."::".$file."::".(isset($pi['extension'])?$pi['extension']:"---")."::".date("F d Y H-i-s", filemtime($_global[signChanell($connect)]["file"].$file))."::".filesize($_global[signChanell($connect)]["file"].$file)."::".$_global[signChanell($connect)]["owner"]."\n";
                }
                $sms = strlen($elem);

            //$sms = getCountFile($root.$cmd[1]);
            $_global[signChanell($connect)]["act"] = 1;
            break;
            # Type::Name::Format::Date::Size::UID
            //echo "";
        }
        case "READ":{
            if ($cmd[1] == "GET"){
                // Юзер запрашивает файлик :(
                // Придеться его отдать...
              if (!file_exists($_global[signChanell($connect)]["file"]) || $_global[signChanell($connect)]["file"] == ""){
                  // Если он вдруг пропал
                  $sms = "";
                  SayoriClean($connect);
                  break;
              }
              //$sms = filesize($_global[signChanell($connect)]["file"]);
              $sms = file_get_contents($_global[signChanell($connect)]["file"]);
              SayoriClean($connect);
              break;
            }
            if (!isset($cmd[1]) || !file_exists($root.SayoriGetPath($cmd[1]))){
                $sms = FILE_NO_SELECTED;
                SayoriClean($connect);
                break;
            }
            if (is_dir($root.SayoriGetPath($cmd[1]))){
                $sms = FILE_IS_DIR;
                SayoriClean($connect);
                break;
            }
            if (!is_readable($root.SayoriGetPath($cmd[1]))){
                $sms = FILE_ERR_NO_READ;
                SayoriClean($connect);
                break;
            }
            $sms = filesize($root.SayoriGetPath($cmd[1]));
            $_global[signChanell($connect)]["file"] = $root.SayoriGetPath($cmd[1]);
            break;
        }
        case "WRITE":{
            if ($_global[signChanell($connect)]["act"] == 0){
                if (!isset($cmd[1])){
                    $sms = FILE_NO_SELECTED;  // Файл не указан
                    SayoriClean($connect);
                    break;
                } elseif (!file_exists($root.SayoriGetPath($cmd[1]))){
                    // Файл не найден
                    // Попробуем его создать
                    if (!touch($root.SayoriGetPath($cmd[1]))) {
                        $sms = FILE_NO_CREATE;
                        SayoriClean($connect);
                        break;
                    }
                }
                if (!is_readable($root.SayoriGetPath($cmd[1]))){
                    $sms = FILE_ERR_NO_WRITE;
                    SayoriClean($connect);
                    break;
                }
                $_global[signChanell($connect)]["file"] = $root.SayoriGetPath($cmd[1]);
                $sms = SUCCESS;
                $_global[signChanell($connect)]["act"] = 1;
                break;
            } elseif ($_global[signChanell($connect)]["act"] == 1){
                if (!isset($cmd[1])){
                    $sms = ARG1_NO_SPECIFIED;  // Аргумент не указан
                    SayoriClean($connect);
                    break;
                }
                $_global[signChanell($connect)]["bytes"] = intval($cmd[1]);
                if ($_global[signChanell($connect)]["bytes"] < 0){
                    $sms = FILE_ERR_BYTE_ZERO;
                    SayoriClean($connect);
                    break;
                }
                $_global[signChanell($connect)]["act"] = 9999;
                $sms = SUCCESS;
                break;
            }
        }
        default:{
            $sms = "UNKNOWN";
            break;
        }
    }
    SayoriSet($sms."\n",$connect);
    echo "< ".$sms.PHP_EOL.PHP_EOL;
}


$_global = [];

$connects = array();
echo "Started tcp://0.0.0.0:10000".PHP_EOL;
while (true) {
    //echo "Waiting...".PHP_EOL;
    //формируем массив прослушиваемых сокетов:
    $read = $connects;
    $read []= $socket;
    $write = $except = null;

    if (!stream_select($read, $write, $except, null)) {//ожидаем сокеты доступные для чтения (без таймаута)
        //echo "NO SOCKET SELECT...".PHP_EOL;
        break;
    }

    //var_dump($read);

    if (in_array($socket, $read)) {//есть новое соединение
        //принимаем новое соединение и производим рукопожатие:
        //stream_set_blocking($socket,false);
        //echo "NEW CONNECT...".PHP_EOL;
        $connect = stream_socket_accept($socket, 10);
        if ($connect && $info = handshake($connect,$socket)) {
            //echo "REG...".PHP_EOL;
            $connects[] = $connect;//добавляем его в список необходимых для обработки
            onOpen($connect, $info);//вызываем пользовательский сценарий
        }
        unset($read[ array_search($socket, $read) ]);
    }

    //echo "FOREACH...".PHP_EOL;
    foreach($read as $connect) {//обрабатываем все соединения
        if ($_global[signChanell($connect)]["act"] == 9999){
            $ans = SayoriWriteFile($connect);
            SayoriSet($ans."\n",$connect);
            SayoriClean($connect);
            $_global[signChanell($connect)]["act"] = 0;
        }


        $adata = SayoriGet($connect);
        SayoriHandler($adata["data"],$connect);
        //onMessage($connect, $data,$info);//вызываем пользовательский сценарий
        //echo "".PHP_EOL.PHP_EOL;
    }
}

fclose($server);

function signChanell($connect){
    $address = explode(':', stream_socket_get_name($connect, true)); //получаем адрес клиента
    $data = str_replace(".", "_", $address[0]);
    $data = $data."_".$address[1];
    return $data;
}

function handshake($connect,$socket) {
    global $_global;
    $info = array();

    $address = explode(':', stream_socket_get_name($connect, true)); //получаем адрес клиента
    $info['ip'] = $address[0];
    $info['port'] = $address[1];
    $info['socket'] = $socket;
    $info['sign'] = signChanell($connect);
    $info['file'] = "";
    $info['act'] = 0;
    $info['bytes'] = 0;
    $info['auth'] = false;
    $info['login'] = "";
    $info['pwd'] = "";
    $info['owner'] = "SayoriOS Team";

    $_global[signChanell($connect)] = $info;
    var_dump($_global,$info);
    return $info;
}

function onOpen($connect, $info) {
    //echo "open\n";
    //var_dump($connect,$info);
    //fwrite($connect, encode('Соединение установлено!'));
}

function onClose($connect) {
    //echo "close\n";
    //var_dump($connect);
}

function onMessage($connect, $data,$info) {
    //var_dump($connect,$info);
    SayoriSet($info['socket'],"HI");
    //echo decode($data)['payload'] . "\n";
    //fwrite($connect, encode('так'));
}
