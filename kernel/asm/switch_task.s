/*
 * SayoriOS
 * Переключение задач
 */
.extern		current_thread
.extern		current_proc
.extern		tss


.global		task_switch

task_switch:
            # push %ebx
			# push %esi
			# push %edi
			push %ebp

			pushf
			cli

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
			mov	current_thread, %edx
			mov 12(%edx), %eax
			mov %eax, current_proc

			mov current_proc, %edx

			# Load our page directory address
			mov 12(%edx), %eax
			mov %cr3, %ecx

			cmp %eax, %ecx
			je	.no_addr_switch

			mov %eax, %cr3

            .no_addr_switch:

    		popf
			# sti

            pop %ebp
            # pop %edi
            # pop %esi
			# pop %ebx

			ret
