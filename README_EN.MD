<html><body>
<h1>SayoriOS - Open-source operating system kernel.</h1>
<img src="https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white" alt="C">
<img src="https://github.com/pimnik98/SayoriOS/actions/workflows/SayoriOS-AutoBot-Auto.yml/badge.svg" alt="Workflow">
<p>This is very simple operating-system with <s>well-documented</s> kernel<br>
    To get started with SayoriOS, you need following tools: <a href="https://github.com/pimnik98/SayoriSDK"><b>SayoriSDK</b></a>
</p>
<h1>Screenshots</h1>
<center>
	<img src="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/v0.3.1/1.jpg" alt="v0.3.1 S1">
	<img src="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/v0.3.1/2.jpg" alt="v0.3.1 S2">
	<img src="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/v0.3.1/3.jpg" alt="v0.3.1 S3">
</center><br>
<h1>Progress of development:</h1>
<p>
	❤  [Kernel] Progress: 4 of 7 (57%) [=====-----]<br>
	💫 [Drivers] Progress: 3 of 6 (50%) [=====-----]<br>
	📂 [Files] Progress: 4 of 8 (50%) [=====-----]<br>
	🌐 [Network] Progress: 0 of 1 (0%) [----------]<br>
	🔌 [Other] Progress: 2 of 5 (40%) [====------]<br>
	<img src="http://forum.glark.ru/smiles.lm?id=38" alt="*"> <a href="https://raw.githubusercontent.com/pimnik98/SayoriOS/screens/screens/soul.png">Click here</a> to view roadmap (in Russian)<br>
</p>
<h1>System reqirements:</h1>
<p>
	* Processor: 1 core х 1 GHz<br>
	* RAM: 32 МB<br>
	* Serial ports: 2<br>
	* Video: 4 MB<br>
</p>
<h1>Testing</h1>
<p>To try SayoriOS, you can <a href="https://github.com/pimnik98/SayoriOS/releases">download altest release</a>.</p>
<h1>Contributing</h1>
<p>
	1. Register your own GitHub account<br>
	2. What you want:<br>
	2.1. Test:<br>
	2.1.1. You can test SayoriOS with QEMU, VirtualBox or other emulators (or even in real machines). You can create <a href="https://github.com/pimnik98/SayoriOS/issues"><b>Issue</b></a><br>
	<br>
	2.2. Develop:<br>
	2.2.1. Make a fork of this project<br>
	2.2.2. Make you changes (you can make them in separate branch)<br>
	2.2.3. You MUST check your changes (compile, run, test) before openining a Pushing to your forked repository.<br>
	2.2.4. Push your changes and open a <a href="https://github.com/pimnik98/SayoriOS/pulls"><b>Pull Request</b></a>.<br>
	2.2.5. Don't forget about Code Style and Pull Request Style in <b>STYLE.md</b>
	<br>
	Please, star our project <img src="http://forum.glark.ru/smiles.lm?id=32" alt=":)">
</p>
<h1>Building a kernel</h1>
<p>
	You need any <b>Linux distribution</b> (NDRAEY uses Debian 11)<br>
	Install dependcies:<br>
	<b><i>sudo apt-get install python3 clang llvm grub-pc-bin xorriso mtools lld git fasm make</b></i><br><br>
	And testing software:<br>
	<b><i>sudo apt-get install libvirt-daemon libvirt-clients bridge-utils virt-manager qemu-kvm qemu virt-manager</b></i><br><br>
	<br>
	Getting ready to build:<br>
	1. Clone a repository:<br>
	<b><i>git clone https://github.com/pimnik98/SayoriOS.git</b></i><br><br>
	2. Enter directory<br>
	<b><i>cd SayoriOS</b></i><br><br>
	3. Compile with `make` command<br>
	<b><i>make</b></i><br><br>
	4. Run QEMU<br>
	<b><i>make bir</b></i></p>
<h1>Makefile commands:</h1>
<p>
	<b><i>make</b></i> - Just build `kernel.elf` file<br>
	<b><i>make run</b></i> - Just run QEMU emulator (logging to Qemu.log file)<br>
	<b><i>make runlive</b></i> - Run QEMU with logging into stdio<br>
	<b><i>make uefi</b></i> - Build UEFI ISO image<br>
	<b><i>make geniso</b></i> - Build BIOS ISO image<br>
	<b><i>make genuefi</b></i> - Build UEFI ISO image (Deprecated)<br>
	<b><i>make clean</b></i> - Clean build root<br>
	<b><i>make clean-objs</b></i> - Delete all `*.o` files<br>
	<b><i>make link</b></i> - Run linker<br>
	<b><i>make bir</b></i> - Make everything: build, make ISO, run<br>
</p>
<h1>General contributors:</h1>
<p>
	* <a href="https://github.com/pimnik98">Nikita Piminoff</a><br>
	* <a href="https://github.com/NDRAEY">Drew Pavlenko</a><br><br>
</p>
<p>
	Check out our <a href="https://vk.com/sayorios">VK group</a><br>
</p>
<p>
<b><i>Cool guys who star this project</i></b>
<p>
<img src="https://reporoster.com/stars/pimnik98/SayoriOS"/>
</p>
</p>
<p>
<B><I>Cool guys who CONTRIBUTE TO THIS PROJECT</I></B>
<p>
<img src="https://reporoster.com/forks/pimnik98/SayoriOS"/>
</p>
</p>
</body></html>

