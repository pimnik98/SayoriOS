/*
 * SayoriOS
 * Переключение задач
 */
.extern		current_thread
.extern		tss


.global		task_switch

task_switch:

			push	%ebp
			
			pushf
			cli
			
			mov	current_thread, %edx
			mov	%esp, 28(%edx)
			
			mov	4(%edx), %ecx
			mov %ecx, current_thread
			
			mov current_thread, %edx
			mov 28(%edx), %esp
			
			mov 40(%edx), %eax			
			mov $tss, %edx				
			mov %eax, 4(%edx)			
			
			popf					
			
			pop		%ebp			
			ret
