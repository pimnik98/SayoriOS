/*
 * SayoriOS
 * Переключение задач
 */
.extern		current_thread
.extern		current_proc
.extern		tss

.global		task_switch

task_switch:
			cli

			pushf

            push %ebx
			push %esi
			push %edi
			push %ebp

			# Save current thread
			mov	current_thread, %edx

			# Save current thread's stack
			mov	%esp, 28(%edx)

			# current_thread.list_item.next is next entry in list
			mov	4(%edx), %ecx

			# Set current_thread to next entry
			mov %ecx, current_thread

			# Load thread's stack
			mov current_thread, %edx
			mov 28(%edx), %esp

			# Load stack_top to tss
			mov 40(%edx), %eax			
			mov $tss, %edx
			mov %eax, 4(%edx)

            # WARNING: THIS CODE IS MAY BE BUGGY (NOT TESTED)
            # IF YOU HAVING PROBLEMS WITH THIS CODE, PLEASE, REPORT IT TO NDRAEY
            # OR REMOVE THESE FOKEN LINES UNTIL THE `popf` INSTRUCTION

			# Load process' page directory
			# Load our process structure
			mov	current_thread, %ebx
			mov 12(%ebx), %eax
			mov %eax, current_proc

			mov current_proc, %ebx

			# Load our page directory address
			mov 12(%ebx), %ebx

			mov %cr3, %eax
			cmp %eax, %ebx

			je .no_switch_pd

			mov %ebx, %cr3

			.no_switch_pd:

            pop %ebp
            pop %edi
            pop %esi
			pop %ebx

    		popf

			ret
