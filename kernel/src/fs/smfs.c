/**
 * @file fs/fat12.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Файловая система SMFS (Sayori Medium File System)
 * @version 0.3.3
 * @date 2023-07-28
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <kernel.h>
#include <io/ports.h> 

#include <fs/smfs.h>

#define FLOPPY_SIZE 1474559	///< Размер Floppy
SMFS_BOOT_SECTOR boot[16] = {0};
SMFS_Elements elements[8196] = {0};
SMFS_PACKAGE pkg[8196] = {0};

SMFS_PACKAGE _smfs_getInfoPackAge(int Address){
	SMFS_PACKAGE* pack = kmalloc(sizeof(SMFS_PACKAGE));
	
	if (pack == -1){
		qemu_log("KMALLOC ERROR!");
		pack[0].Status = SMFS_PACKAGE_UNKNOWN;
		return pack[0];
	}
	
	int read = _FloppyRead(0, &pack[0], Address, sizeof(SMFS_PACKAGE));
	
	return pack[0];
}

SMFS_Elements _smfs_getInfoElem(unsigned int Index){
	SMFS_Elements* elem = kmalloc(sizeof(SMFS_Elements));
	
	if (!elem){
		qemu_log("KMALLOC ERROR!");
		elem[0].Attr = SMFS_TYPE_UNKNOWN;
		return elem[0];
	}
	
	int read = _FloppyRead(0,&elem[0],sizeof(SMFS_BOOT_SECTOR)+(sizeof(SMFS_Elements)*Index),sizeof(SMFS_Elements));

	return elem[0];
}

uint32_t _smfs_foundFile(char* path){
	char ser[2] = {0x5C,0};
	
	char_replace(0x2F,0x5C,path);	//< Превращает все (0x2F)/ в (0x5C)\\

	strtolower(path);	//! Нет поддержки русских букв!
	
	char tname[32] = {0};
	qemu_log("[smfs] found file: %s",path);
	
	uint32_t dir_c = str_cdsp(path,ser);
	char* out[128] = {0};
	
	str_split(path,out,ser);

	qemu_log("[smfs] found: %d",dir_c);
	uint32_t dir = 0;
	for(int i = 0; i < dir_c; i++){
		for (int a = 0; a < boot[1].MaximumElems; a++){
			SMFS_Elements elem = _smfs_getInfoElem(a);

			if (elem.Attr == SMFS_TYPE_UNKNOWN) continue;	///< Пропускаем, тк битый файл
			if (elem.Attr == SMFS_TYPE_DELETE) continue;	///< Пропускаем, тк удален файл
			if (elem.Dir != dir) continue; ///< Пропускаем, тк не рут папка

			memcpy(tname,elem.Name,32);
			strtolower(tname);	//! Нет поддержки русских букв!
			if (strcmp(tname,out[0]) == 0 && i+1 == dir_c){
				qemu_log("OUT");
				return i;
			} else if (strcmp(tname,out[0]) == 0) {
				qemu_log("Next door: %d",a);
				dir = a;
				continue;
			} else {
				/// ????
			}


			qemu_log("[%i] [%s == %s] => %d",i,tname,out[0],strcmp(tname,out[0]));

			__com_formatString(0x3f8," |--- Index          | %x\n",elem.Index);
			__com_formatString(0x3f8," |--- Attr           | %x\n",elem.Attr);
			__com_formatString(0x3f8," |--- Size           | %x\n",elem.Size);
			__com_formatString(0x3f8," |--- TimeCreateHIS  | %x\n",elem.TimeCreateHIS);
			__com_formatString(0x3f8," |--- TimeCreateDate | %x\n",elem.TimeCreateDate);
			__com_formatString(0x3f8," |--- TimeAccess     | %x\n",elem.TimeAccess);
			__com_formatString(0x3f8," |--- Point          | %x\n",elem.Point);
			__com_formatString(0x3f8," |--- Dir            | %x\n",elem.Dir);
			__com_formatString(0x3f8," |--- Name           | %s\n",elem.Name);
			__com_formatString(0x3f8," |--- Size           | %d\n",elem.Size);
		}
		
		qemu_log("[smfs] [%d] %s ",i,out[i]);
	}
	return -1;
}

void _smfs_PackageFix(char* src, int count){
	for (int i = count; i < 9;i++){
		src[i] = 0;
	}
}


uint32_t _smfs_ReadFile(uint32_t Index, char* dst){
	SMFS_Elements elem = _smfs_getInfoElem(Index);
	if (elem.Attr == SMFS_TYPE_UNKNOWN){
		qemu_log("ReadFile Error");
		return -1;
	}
	if (elem.Attr == SMFS_TYPE_DELETE){
		qemu_log("ReadFile Deleted!");
		return -2;
	}
	SMFS_PACKAGE pack = _smfs_getInfoPackAge(elem.Point);
	if (pack.Status == SMFS_PACKAGE_UNKNOWN){
		return -3;
	}
	int seek = 0;
	while(1){
		for (int i=0;i < pack.Length; i++){
			dst[seek] = pack.Data[i];
			seek++;
		}
		if (pack.Next == -1) break;
		pack = _smfs_getInfoPackAge(pack.Next);
	}
	return seek;
}

int _smfs_WritePackage(uint32_t Addr, SMFS_PACKAGE pack) {
	int read = 0;
	read = _FloppyWrite(0,&pack,Addr,sizeof(SMFS_Elements));
	qemu_log("[SMFS] Send to package to %x",Addr);
	__com_formatString(0x3f8," |--- Addr           | %x\n",Addr);
	__com_formatString(0x3f8," |--- Write          | %d\n",read);
	__com_formatString(0x3f8," |--- Data           | %s\n",pack.Data);
	__com_formatString(0x3f8," |--- Status         | %d\n",pack.Status);
	__com_formatString(0x3f8," |--- Length         | %d\n",pack.Length);
	__com_formatString(0x3f8," |--- Next           | %x\n\n",pack.Next);
	return read;
}

int _smfs_DeletePackage(uint32_t Addr){
	if (Addr == 0) return 0;
	SMFS_PACKAGE Free = _smfs_getInfoPackAge(Addr);
	if (Free.Status == SMFS_PACKAGE_FREE) return 1;
	memset(Free.Data, 0, 9);
	Free.Length = 0;
	Free.Next = -1;
	Free.Status = SMFS_PACKAGE_FREE;
	
	return _smfs_WritePackage(Addr,Free);
	
}

uint32_t _smfs_DeleteFile(uint32_t Index){
	SMFS_Elements elem = _smfs_getInfoElem(Index);
	if (elem.Attr == SMFS_TYPE_UNKNOWN){
		qemu_log("File Error");
		return -1;
	}
	if (elem.Attr == SMFS_TYPE_DELETE){
		qemu_log("Already Deleted!");
		return -2;
	}
	if (elem.Point != 0){
		/// Зачищаем блоки, только если точка монтирования указана
		SMFS_PACKAGE pack = _smfs_getInfoPackAge(elem.Point);
		if (pack.Status == SMFS_PACKAGE_UNKNOWN){
			return -3;
		}
		int countPack = (elem.Size == 0?1:(elem.Size/8)+1);
		uint32_t *pkg_addr = kmalloc(sizeof(uint32_t)*(countPack+1));
		int seek = 1;
		pkg_addr[0] = elem.Point;
		while(1){
			if (pack.Next == -1) break;

			pkg_addr[seek] = pack.Next;
			pack = _smfs_getInfoPackAge(pack.Next);
			seek++;
			if (pack.Status == SMFS_PACKAGE_UNKNOWN){
				qemu_log("CRITICAL GET ERROR");
				return -4;
			}
		}
		
		for (int i = 0; i < countPack; i++) {
			qemu_log("Delete %x",pkg_addr[i]);
			_smfs_DeletePackage(pkg_addr[i]);
		}
	}
	char eName[32] = {0};
	elements[0].Attr = SMFS_TYPE_DELETE;
	elements[0].Size = 0;
	elements[0].TimeCreateHIS = 0;
	elements[0].TimeCreateDate = 0;
	elements[0].TimeAccess = 0;
	elements[0].Dir = 0;
	elements[0].Point = 0;
	memcpy((void*) elements[0].Name, eName, 32);
	int read = _FloppyWrite(0,&elements[0],sizeof(SMFS_BOOT_SECTOR)+(sizeof(SMFS_Elements)*Index),sizeof(SMFS_Elements));
	if (read != sizeof(SMFS_Elements)){
		return -5;
	}
	qemu_log("Deleted file complete! %d",read);
	return 1;
}

uint32_t _smfs_FreePackAge(int skip) {
	int adr = 0;
	
	for (int i = 0; i < boot[1].MaxPackage; i++){
		adr = (sizeof(SMFS_BOOT_SECTOR) + (sizeof(SMFS_Elements) * boot[1].MaximumElems) + (i * sizeof(SMFS_PACKAGE)));

		SMFS_PACKAGE pack = _smfs_getInfoPackAge(adr);

		qemu_log("[%d] Test PackAge");
		
		__com_formatString(0x3f8," |--- Addr           | %x\n",adr);
		__com_formatString(0x3f8," |--- Data           | %s\n",pack.Data);
		__com_formatString(0x3f8," |--- Status         | %d\n",pack.Status);
		__com_formatString(0x3f8," |--- Length         | %d\n",pack.Length);
		__com_formatString(0x3f8," |--- Next           | %x\n\n",pack.Next);
		
		if (pack.Status == SMFS_PACKAGE_FREE && skip !=0){
			skip--;
			continue;
		}
		
		if (pack.Status == SMFS_PACKAGE_FREE) return adr;
	}
	
	return -1;
}

int _smfs_WriteFile(char* path, char* src){
	// Тут мы типа должны найти файл и обратиться по позиции указанном в Pointer
	
	int found = _smfs_foundFile(path);
	if (found == -1){
		return -1;
	}
	int src_size = strlen(src);
	int src_seek = 0;
	int currentAddr = 0;
	int read = 0;
	char src_buf[8] = {0};
	int countPack = (src_size == 0?1:(src_size/8)+1);
	
	qemu_log("[WriteFile]\n * src_size: %d\n * countPack : %d\n * src: %s\n",src_size,countPack,src);

	SMFS_PACKAGE *pkg_free = kmalloc(sizeof(SMFS_PACKAGE));
	uint32_t *pkg_addr = kmalloc(sizeof(uint32_t)*(countPack+1));
	// Выделяем память для структур и записи
	if (pkg_free == -1 || pkg_addr == -1){
		qemu_log("KMALLOC ERROR");
		return -1;
	}

	for (int i = 0; i < countPack; i++) {
		pkg_addr[i] = _smfs_FreePackAge(i);
		
		if (pkg_addr[i] == -1) {
			qemu_log("NO FREE PACKAGE!!!");
			return -1;
		}
		
		qemu_log("Found %x FREE PACKAGE!", pkg_addr[i]);
	}
	
	for (int a = 0; a <= src_size; a++){
		if (src_seek == 8){
			// Buffer
			_smfs_PackageFix(src_buf,src_seek);
			
			qemu_log("Buffer: %s",src_buf);

			memcpy((void*) pkg_free[0].Data, src_buf, 9);
			
			pkg_free[0].Length = 8;
			pkg_free[0].Next = pkg_addr[currentAddr+1];
			pkg_free[0].Status = SMFS_PACKAGE_READY;
			
			_smfs_WritePackage(pkg_addr[currentAddr], pkg_free[0]);
			
			currentAddr++;
			src_seek = 0;
			a--;
			//src_buf = {0};
		} else if (a == (src_size)){
			_smfs_PackageFix(src_buf,src_seek);
			
			qemu_log("EOL | Buffer: %s",src_buf);
			memcpy((void*) pkg_free[0].Data, src_buf, 8);
			
			pkg_free[0].Length = src_seek;
			pkg_free[0].Next = -1;
			pkg_free[0].Status = SMFS_PACKAGE_READY;
			
			_smfs_WritePackage(pkg_addr[currentAddr],pkg_free[0]);
			
			break;
		} else {
			src_buf[src_seek] = src[a];
			src_seek++;
		}
		//pkg_free[0].Status = SMFS_PACKAGE_READY;
		//pkg_free[0].Status = SMFS_PACKAGE_READY;
	}
	
	qemu_log("Write complete!");
	qemu_log("[+] Package to write:\n");
	
	for (int i = 0; i < countPack; i++){
		__com_formatString(0x3f8," |--- Addr           | %x\n",pkg_addr[i]);
	}
	return src_size;
	
	//uint32_t fpa = _smfs_FreePackAge();
}

int _smfs_createFile(uint32_t dir,const char* name,int type){
	uint32_t Index = -1;
	SMFS_Elements elem;
	for (int a = 0; a < boot[1].MaximumElems; a++){
		SMFS_Elements elem = _smfs_getInfoElem(a);
		if (elem.Attr == SMFS_TYPE_UNKNOWN) continue;
		if (elem.Attr == SMFS_TYPE_DELETE){
			Index = a;
		}
	}
	if (Index == -1){
		return 0;
	}
	elem.Attr = type;
	elem.Size = 0;
	elem.TimeCreateHIS = 0;
	elem.TimeCreateDate = 0;
	elem.TimeAccess = 0;
	elem.Dir = dir;
	elem.Point = 0;
	memcpy((void*) elem.Name, name, 32);
	int read = _FloppyWrite(0, &elem, sizeof(SMFS_BOOT_SECTOR) + (sizeof(SMFS_Elements) * Index), sizeof(SMFS_Elements));
	qemu_log("[SMFS] FDA WRITE:%d | Index: %d\n", read, Index);
	return 1;
}

void _smfs_format(int MaxFiles){
	int read = 0,seek = 0;
	int sizeFiles = ((sizeof(SMFS_Elements))*MaxFiles);
	int freeSpa = (FLOPPY_SIZE-(sizeFiles)-(sizeof(SMFS_BOOT_SECTOR)));
	int allPkg = (freeSpa/(sizeof(SMFS_PACKAGE)));
	
	qemu_log("[SMFS] Formatting FDA STARTED!");
	qemu_log("[SMFS] FLOPPY_SIZE = %d",FLOPPY_SIZE);
	qemu_log("[SMFS] BOOT_SIZE = %d",(sizeof(SMFS_BOOT_SECTOR)));
	qemu_log("[SMFS] Max Elems = %d | %d",MaxFiles,sizeFiles);
	qemu_log("[SMFS] FREE SPA = %d",freeSpa);
	qemu_log("[SMFS] Max Package = %d",allPkg);
	qemu_log("[SMFS] We overwrite on our own BOOT SECTOR");
	
	char oem[8] = {'S','A','Y','O','R','I','O','S'};
	char label[11] = {'S','M','F','S',' ',' ',' ',' ',' ',' ',0};
	char fsid[8] = {'S','M','F','S','1','.','0',' '};
	
	boot[0].magic1 = 0x7246;
	boot[0].magic2 = 0xCAFE;
	boot[0].MaximumElems = MaxFiles;
	boot[0].MaxPackage = allPkg;
	
	memcpy((void*) boot[0].oem_name, oem, 8);
	memcpy((void*) boot[0].volume_label, label, 11);
	memcpy((void*) boot[0].fsid, fsid, 8);
	
	read = _FloppyWrite(0,&boot[0],0,sizeof(SMFS_BOOT_SECTOR));
	
	qemu_log("[SMFS] FDA WRITE:%d\n",read);
	
	if (read == -1){
		qemu_log("[SMFS] FATAL FORMAT!");
		return;
	}
	
	seek = sizeof(SMFS_BOOT_SECTOR);
	qemu_log("[SMFS] Create markup structures for files and folders");
	
	char eName[32] = {0};
	elements[0].Attr = SMFS_TYPE_DELETE;
	elements[0].Size = 0;
	elements[0].TimeCreateHIS = 0;
	elements[0].TimeCreateDate = 0;
	elements[0].TimeAccess = 0;
	elements[0].Dir = 0;
	elements[0].Point = 0;
	
	memcpy((void*) elements[0].Name, eName, 32);
	
	for (int i = 0; i < MaxFiles; i++) {
		TE_DrawMessageBox("Разметка макета", "Это длительный процесс...", 2, getScreenWidth() / 4, 100, 0, i, MaxFiles);
		elements[0].Index = i;
		read = _FloppyWrite(0,&elements[0],seek,sizeof(SMFS_Elements));
		qemu_log("[SMFS] FDA WRITE:%d | I: %d | In: %x\n",read,i,seek);
		seek += sizeof(SMFS_Elements);
		
		punch();
	}
	
	qemu_log("[SMFS] Now we write packages");
	
	char eData[8] = {0};
	memcpy((void*) pkg[0].Data, eData, 8);
	pkg[0].Length = 0;
	pkg[0].Next = -1;
	pkg[0].Status = SMFS_PACKAGE_FREE;

	
	//char* outmark = kmalloc(sizeof(SMFS_PACKAGE)*allPkg);
	
	//int32_t* outmark = memoryMulti(pkg[0],5,allPkg);	

	//memmove(outmark, pkg[0], allPkg);
	//read = _FloppyWrite(0,outmark,seek,5*allPkg);
	//qemu_log("[PRINT] \n%s\n",outmark);
	//qemu_log("[SMFS] FDA WRITE:%d | Count: %d | In: %x\n",read,sizeof(SMFS_PACKAGE)*allPkg,seek);
	
	for (int i = 0; i < allPkg; i++){
		TE_DrawMessageBox("Разметка пакета", "Последний этап", 2, getScreenWidth() / 4, 400, 0, i, allPkg);
		read = _FloppyWrite(0,&pkg[0],seek,sizeof(SMFS_PACKAGE));
		qemu_log("[SMFS] FDA WRITE:%d | I: %d | In: %x\n",read,i,seek);
		seek += sizeof(SMFS_PACKAGE);
		punch();
	}

	
}

void _smfs_write(void* data,size_t size){
	qemu_log("WRITE!");
	for (int i = 0; i < size; i++){
		__com_formatString(0x3f8,"[%d] %d | %c\n",i,((int*)data)[i],((char*)data)[i]);
	}
}

void bKbMb(int bytes){
	qemu_log("[SMFS] %d b | %d Kb | %d Mb",bytes,bytes/1024,(bytes/1024)/1024);
}

void _smfs_printFF(){
	int read = 0;
	qemu_log("[Print Files & Folder]");
	qemu_log("[SMFS] Emelent size: %d",sizeof(SMFS_Elements));
	qemu_log("[SMFS] MaximumElems: %d",boot[1].MaximumElems);
	for (int i = 0; i < boot[1].MaximumElems; i++){
		read = _FloppyRead(0,&elements[1],sizeof(SMFS_BOOT_SECTOR)+(sizeof(SMFS_Elements)*i),sizeof(SMFS_Elements));
		qemu_log("[%d] READ: %d | Pos: %d",i,read,sizeof(SMFS_BOOT_SECTOR)+(sizeof(SMFS_Elements)*i));
		__com_formatString(0x3f8," |--- Index          | %x\n",elements[1].Index);
		__com_formatString(0x3f8," |--- Attr           | %x\n",elements[1].Attr);
		__com_formatString(0x3f8," |--- Size           | %x\n",elements[1].Size);
		__com_formatString(0x3f8," |--- TimeCreateHIS  | %x\n",elements[1].TimeCreateHIS);
		__com_formatString(0x3f8," |--- TimeCreateDate | %x\n",elements[1].TimeCreateDate);
		__com_formatString(0x3f8," |--- TimeAccess     | %x\n",elements[1].TimeAccess);
		__com_formatString(0x3f8," |--- Point          | %x\n",elements[1].Point);
		__com_formatString(0x3f8," |--- Dir            | %x\n",elements[1].Dir);
		__com_formatString(0x3f8," |--- Name           | %s\n",elements[1].Name);
		__com_formatString(0x3f8,"\n",elements[1].Size);
	}
	//elements[0];
}

void _smfs_init(){
	qemu_log("[SMFS] Init...");
	qemu_log("%d | %x",0x167FFF,0x167FFF);
	qemu_log("[SMFS] В пизду");return;
	qemu_log("[SMFS] Boot Sector size: %d",sizeof(SMFS_BOOT_SECTOR));
	qemu_log("[SMFS] Package size: %d",sizeof(SMFS_PACKAGE));
	qemu_log("[SMFS] Emelent size: %d",sizeof(SMFS_Elements));
	qemu_log("[SMFS] Emelents group size: %d",sizeof(SMFS_Elements)*256);
	qemu_log("[SMFS] Free Space: %d",FLOPPY_SIZE-(sizeof(SMFS_Elements)*256)-sizeof(SMFS_BOOT_SECTOR));
	bKbMb(FLOPPY_SIZE-(sizeof(SMFS_Elements)*256)-sizeof(SMFS_BOOT_SECTOR));
	qemu_log("[SMFS] Count package: %d",(FLOPPY_SIZE-(sizeof(SMFS_Elements)*256)-sizeof(SMFS_BOOT_SECTOR))/sizeof(SMFS_PACKAGE));
	bKbMb((FLOPPY_SIZE-(sizeof(SMFS_Elements)*256)-sizeof(SMFS_BOOT_SECTOR))/sizeof(SMFS_PACKAGE));
	

	//_smfs_format(256); return;
	int read = 0;
	read = _FloppyRead(0,&boot[1],0,39);
	qemu_log("[SMFS] Read:%d",read);
	//_smfs_write(boot,sizeof(SMFS_BOOT_SECTOR));
	//read = _FloppyWrite(0,boot,0,sizeof(SMFS_BOOT_SECTOR));
	//qemu_log("[SMFS] WRITE:%d\n%s",read);
	qemu_log("[SMFS] Check BootSector:");
	__com_formatString(0x3f8," |--- magic1       | %x\n",boot[1].magic1);
	__com_formatString(0x3f8," |--- magic2       | %x\n",boot[1].magic2);
	__com_formatString(0x3f8," |--- MaximumElems | %d\n",boot[1].MaximumElems);
	__com_formatString(0x3f8," |--- MaxPackage   | %d\n",boot[1].MaxPackage);
	__com_formatString(0x3f8," |--- OEM          | %s\n",boot[1].oem_name);
	__com_formatString(0x3f8," |--- Label        | %s\n",boot[1].volume_label);
	__com_formatString(0x3f8," |--- FSID         | %s\n",boot[1].fsid);
	// FloppySize - 1509949
	qemu_log("[SMFS] %d b | %d Kb | %d Mb",FLOPPY_SIZE,FLOPPY_SIZE/1024,(FLOPPY_SIZE/1024)/1024);
	int fadr = _smfs_foundFile("\\TeST.TxT");
	if (fadr == -1){
		qemu_log("Test.txt No found");
	} else {
		SMFS_Elements elem = _smfs_getInfoElem(fadr);
		if (elem.Attr == SMFS_TYPE_UNKNOWN){
			qemu_log("SMFS_TYPE_ERROR");
		} else {
			char* data = kmalloc(elem.Size);
			int asd = _smfs_ReadFile(elem.Index,data);
			qemu_log("READ: %d | \n%s",asd,data);
		}
	}
	//qemu_log("Delete file? %d",_smfs_DeleteFile(0),_smfs_DeleteFile(1),_smfs_DeleteFile(2));
	//_smfs_foundFile("\\SUKA\\BLAT/Poshe.txt");
	//_smfs_foundFile("/SayoriDev/SDK/Chip/ПиздЕц.elf");
	//_smfs_WriteFile(0,"Is im here blyat! Da skolko mozno tvorit' ety dick!");
	//_smfs_FreePackAge(16);
	//_smfs_printFF();
}
