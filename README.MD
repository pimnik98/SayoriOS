<html><body>
<h1>SayoriOS - открытое публичное ядро операционной системы</h1>
<img src="https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white" alt="C">
<img src="https://github.com/pimnik98/SayoriOS/actions/workflows/SayoriOS-AutoBot-Auto.yml/badge.svg" alt="Workflow">
<p>Простая x86 операционная система на языке C с хорошо документированным ядром.<br>
    Для работы с ядром, могут потребоваться инструменты <a href="https://github.com/pimnik98/SayoriSDK"><b>SayoriSDK</b></a>
</p>
<h1>Скриншоты</h1>
<center>
	<img src="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/v0.3.1/1.jpg" alt="v0.3.1 S1">
	<img src="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/v0.3.1/2.jpg" alt="v0.3.1 S2">
	<img src="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/v0.3.1/3.jpg" alt="v0.3.1 S3">
</center><br>
<h1>Примерный план действий:</h1>
<p>
	Прогресс: <br>
	❤ [Ядро] Выполнено 4 из 7 (57%) [=====-----]<br>
	💫 [Драйвера] Выполнено 3 из 6 (50%) [=====-----]<br>
	📂 [Файлы] Выполнено 4 из 8 (50%) [=====-----]<br>
	🌐 [Сеть] Выполнено 0 из 1 (0%) [----------]<br>
	🔌 [Прочие] Выполнено 2 из 5 (40%) [====------]<br>
	<img src="http://forum.glark.ru/smiles.lm?id=38" alt="*"> <a href="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/soul.png">Нажав сюда</a>, вы увидите изображение, где отображены примерные планы на ядро.<br>
	<b>Обновление данного изображения, будет только при следующем релизе.</b> или <a href="https://sayorios.piminoff.ru/page/1-plans.html">нажмите сюда</a>, для просмотра на сайте
</p>
<h1>Системные требования:</h1>
<p>
	Ядро не требует много ресурсов, но выделить и больше.<br>
	* Процессор: 1 ядро х 1 ГГц c поддержкой x86<br>
	* ОЗУ: 16 МБ<br>
	* Жесткий диск: (пока не требуется, тк нет драйверов для работы)<br>
	* COM-порты: 2 шт. (для отладки)<br>
	* Видеоадаптер: 4 MB+<br>
</p>
<h1>Тестирование:</h1>
<p>Для тестирования ОС, вы можете <a href="https://github.com/pimnik98/SayoriOS/releases">скачать последний релиз</a>.</p>
<h1>Вклад пользы в ядро:</h1>
<p>
	1. Заведите аккаунт на GitHub (если еще не сделали)<br>
	2. Что можно сделать:<br>
	2.1. Если, не умеете программировать:<br>
	2.1.1. Протестируйте ядро, ищите баги, и создавайте <a href="https://github.com/pimnik98/SayoriOS/issues"><b>Issues</b></a> (я называю их Запросами)<br>
	2.1.2. К Запросу желательно прикладывать информацию:<br>
	2.1.2.1. Об использованной версии ядра<br>
	2.1.2.2. Об действиях которые вы сделали, что привело к паданию ядра<br>
	2.1.2.3. Что вы хотите в этом пункте увидить?<br>
	2.1.2.4. Добавите скриншот и файл лога от ядра.<br>
	<br>
	2.2. Если, умеете программировать и знаете как можно решить часть проблем:<br>
	2.2.1. Сделать Форк проекта<br>
	2.2.2. Внести свои изменения<br>
	2.2.3. Проверить свои изменения у себя (скомпилировать, получить желаемый результат)<br>
	2.2.4. Отправить <a href="https://github.com/pimnik98/SayoriOS/pulls"><b>Pull Request </b></a> со своими изменениями в ядро.<br>
	2.2.5. Также не забывайте про стиль кода и коммитов, он указан в <b>STYLE.md</b>
	<br>
	P.S. Можно поставить еще звезду проекту, вам несложно, нам (команде) приятно <img src="http://forum.glark.ru/smiles.lm?id=32" alt=":)">
</p>
<h1>Сборка ядра:</h1>
<p>
	Для компиляции вам потребуется операционная система <b>Linux</b>, в которой предустановлено ПО для разработки.<br>
	Установить можно следующей командой:<br>
	<b><i>sudo apt-get install python3 clang llvm grub-pc-bin xorriso mtools lld git fasm</b></i><br><br>
	А также надо установить софт для тестирования в среде <b>Linux</b><br>
	<b><i>sudo apt-get install libvirt-daemon libvirt-clients bridge-utils virt-manager qemu-kvm qemu virt-manager</b></i><br><br>
	<br>
	Приступаем к компиляции:<br>
	1. Клонируем репо:<br>
	<b><i>git clone https://github.com/pimnik98/SayoriOS.git</b></i><br><br>
	2. Переходим к папке<br>
	<b><i>cd SayoriOS</b></i><br><br>
	3. Команда для компилирования<br>
	<b><i>make</b></i><br><br>
	4. Запуск<br>
	<b><i>make bir</b></i></p>
<h1>Список команд для Make:</h1>
<p>
	<b><i>make</b></i> - Классическая сборка<br>
	<b><i>make run</b></i> - Обычный запуск в QEMU с выводом отладки в Qemu.log <br>
	<b><i>make runlive</b></i> - Обычный запуск в QEMU с выводом отладки в консоль <br>
	<b><i>make uefi</b></i> - Сборка образа в UEFI <br>
	<b><i>make geniso</b></i> - Классическая сборка ISO <br>
	<b><i>make genuefi</b></i> - Сборка ядра с UEFI <br>
	<b><i>make clean</b></i> - Удаляет ISO в папке <br>
	<b><i>make clean-objs</b></i> - Удаляет все скомпилированные файлы <br>
	<b><i>make link</b></i> - Линк файлов ядра <br>
	<b><i>make bir</b></i> - Сборка ISO и запуск ядра <br>
</p>
<h1>Список людей, которые поддерживают ядро:</h1>
<p>
	* <a href="https://github.com/pimnik98">Nikita Piminoff</a><br>
	* <a href="https://github.com/NDRAEY">Drew Pavlenko</a><br><br>
	* <a href="https://vk.com/sayorios">SayoriOS в VK</a><br>
</p>
<h1>Отказ от ответственности:</h1>
<p>
	SayoriOS это не дистрибутив linux, это новый проект который не имеет за собой компании или организации которая могла бы дать гарантий. <br>
	Ядро SayoriOS имеет открытый исходный код, вы можете сами удостовериться в отсутствии вредоносного ПО изучая файлы этого репозитория. <br>
	При использовании материалов вы обязуетесь соблюдать авторские права.<br>
	<b>Ни Я, ни НАША КОМАНДА</b> не несут ответственности за причиненный ущерб.<br>
	Запуск ядра возможен на реальном оборудовании, были тесты и на некоторых устройствах оно реально запустилось, хоть и с ограничениями.<br>
	<b>Используйте на свой страх и риск.</b>
</p>
</body></html>

