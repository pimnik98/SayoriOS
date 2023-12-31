var arr = array();
array_push(arr,"test","ellynet");
array_push(arr,"sayorios","presents");
array_push(arr,"current",3);
console_log(array_getByKey(arr, "test"));   // ellynet
console_log(array_getByIndex(arr, 1));      // presents
console_log(array_length(arr));             // 3


var arr2 = array();
array_push(arr2,"test","sayoridev");
array_push(arr2,"current",3);
array_push(arr2,"helpme","error");

array_editByID(arr2,2,"By Edit Prodaction");
array_editByID(arr2,7,"Lol");

var arr3 = array_diff(arr,arr2);
alert("\nArray1:");
array_dump(arr);
alert("\nArray2:");
array_dump(arr2);
alert("\nArray3:");
array_dump(arr3);