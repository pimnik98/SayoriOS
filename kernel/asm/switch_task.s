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

            # DO WE NEED TO SAVE AND RESTORE REGISTERS?

            # IN
			# EAX at [EBP + 32]
            # EBX at [EBP + 20]
            # ECX at [EBP + 28]
            # EDX at [EBP + 24]
            # EBP at [EBP + 12]
            # ESI at [EBP + 8]
            # EDI at [EBP + 4]

            # OUT
			# EAX at [44]
			# EBX at [48]
			# ECX at [52]
			# EDX at [56]
			# ESI at [60]
            # EDI at [64]
            # EBP at [68]

			pushf
			cli

			#mov	32(%esp), %ebx
			#mov	32(%esp), %ecx
			#mov	32(%esp), %edx
			#mov	32(%esp), %esi
			#mov	32(%esp), %edi

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
			mov %eax, %cr3

    		popf

            pop %ebp
            # pop %edi
            # pop %esi
			# pop %ebx

			ret
