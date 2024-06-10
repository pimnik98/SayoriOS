# Code from https://github.com/llvm-mirror/compiler-rt/blob/master/lib/builtins/i386/

.text
.balign 4
.global __udivdi3
__udivdi3:
	pushl		%ebx
	movl	 20(%esp),			%ebx	# Find the index i of the leading bit in b.
	bsrl		%ebx,			%ecx	# If the high word of b is zero, jump to
	jz			9f						# the code to handle that special case [9].

	# High word of b is known to be non-zero on this branch

	movl	 16(%esp),			%eax	# Construct bhi, containing bits [1+i:32+i] of b

	shrl		%cl,			%eax	# Practically, this means that bhi is given by:
	shrl		%eax					#
	notl		%ecx					#		bhi = (high word of b) << (31 - i) |
	shll		%cl,			%ebx	#			  (low word of b) >> (1 + i)
	orl			%eax,			%ebx	#
	movl	 12(%esp),			%edx	# Load the high and low words of a, and jump
	movl	  8(%esp),			%eax	# to [1] if the high word is larger than bhi
	cmpl		%ebx,			%edx	# to avoid overflowing the upcoming divide.
	jae			1f

	# High word of a is greater than or equal to (b >> (1 + i)) on this branch

	divl		%ebx					# eax <-- qs, edx <-- r such that ahi:alo = bs*qs + r

	pushl		%edi
	notl		%ecx
	shrl		%eax
	shrl		%cl,			%eax	# q = qs >> (1 + i)
	movl		%eax,			%edi
	mull	 20(%esp)					# q*blo
	movl	 12(%esp),			%ebx
	movl	 16(%esp),			%ecx	# ECX:EBX = a
	subl		%eax,			%ebx
	sbbl		%edx,			%ecx	# ECX:EBX = a - q*blo
	movl	 24(%esp),			%eax
	imull		%edi,			%eax	# q*bhi
	subl		%eax,			%ecx	# ECX:EBX = a - q*b
	sbbl		$0,				%edi	# decrement q if remainder is negative
	xorl		%edx,			%edx
	movl		%edi,			%eax
	popl		%edi
	popl		%ebx
	retl


1:	# High word of a is greater than or equal to (b >> (1 + i)) on this branch

	subl		%ebx,			%edx	# subtract bhi from ahi so that divide will not
	divl		%ebx					# overflow, and find q and r such that
										#
										#		ahi:alo = (1:q)*bhi + r
										#
										# Note that q is a number in (31-i).(1+i)
										# fix point.

	pushl		%edi
	notl		%ecx
	shrl		%eax
	orl			$0x80000000,	%eax
	shrl		%cl,			%eax	# q = (1:qs) >> (1 + i)
	movl		%eax,			%edi
	mull	 20(%esp)					# q*blo
	movl	 12(%esp),			%ebx
	movl	 16(%esp),			%ecx	# ECX:EBX = a
	subl		%eax,			%ebx
	sbbl		%edx,			%ecx	# ECX:EBX = a - q*blo
	movl	 24(%esp),			%eax
	imull		%edi,			%eax	# q*bhi
	subl		%eax,			%ecx	# ECX:EBX = a - q*b
	sbbl		$0,				%edi	# decrement q if remainder is negative
	xorl		%edx,			%edx
	movl		%edi,			%eax
	popl		%edi
	popl		%ebx
	retl


9:	# High word of b is zero on this branch

	movl	 12(%esp),			%eax	# Find qhi and rhi such that
	movl	 16(%esp),			%ecx	#
	xorl		%edx,			%edx	#		ahi = qhi*b + rhi	with	0 ≤ rhi < b
	divl		%ecx					#
	movl		%eax,			%ebx	#
	movl	  8(%esp),			%eax	# Find qlo such that
	divl		%ecx					#
	movl		%ebx,			%edx	#		rhi:alo = qlo*b + rlo  with 0 ≤ rlo < b
	popl		%ebx					#
	retl								# and return qhi:qlo




.text
.balign 4
.global __umoddi3
__umoddi3:
	pushl		%ebx
	movl	 20(%esp),			%ebx
	bsrl		%ebx,			%ecx	
	jz			9f						

	movl	 16(%esp),			%eax	

	shrl		%cl,			%eax	
	shrl		%eax					
	notl		%ecx					
	shll		%cl,			%ebx	
	orl			%eax,			%ebx	
	movl	 12(%esp),			%edx	
	movl	  8(%esp),			%eax	
	cmpl		%ebx,			%edx	
	jae			2f

	divl		%ebx

	pushl		%edi
	notl		%ecx
	shrl		%eax
	shrl		%cl,			%eax	
	movl		%eax,			%edi
	mull	 20(%esp)					
	movl	 12(%esp),			%ebx
	movl	 16(%esp),			%ecx	
	subl		%eax,			%ebx
	sbbl		%edx,			%ecx	
	movl	 24(%esp),			%eax
	imull		%edi,			%eax	
	subl		%eax,			%ecx	

	jnc			1f						
	addl	 20(%esp),			%ebx	
	adcl	 24(%esp),			%ecx	
1:	movl		%ebx,			%eax
	movl		%ecx,			%edx

	popl		%edi
	popl		%ebx
	retl


2:
	subl		%ebx,			%edx	
	divl		%ebx					

	pushl		%edi
	notl		%ecx
	shrl		%eax
	orl			$0x80000000,	%eax
	shrl		%cl,			%eax	
	movl		%eax,			%edi
	mull	 20(%esp)					
	movl	 12(%esp),			%ebx
	movl	 16(%esp),			%ecx	
	subl		%eax,			%ebx
	sbbl		%edx,			%ecx	
	movl	 24(%esp),			%eax
	imull		%edi,			%eax	
	subl		%eax,			%ecx	

	jnc			3f						
	addl	 20(%esp),			%ebx	
	adcl	 24(%esp),			%ecx	
3:	movl		%ebx,			%eax
	movl		%ecx,			%edx

	popl		%edi
	popl		%ebx
	retl


9:	
	movl	 12(%esp),			%eax
	movl	 16(%esp),			%ecx	
	xorl		%edx,			%edx	
	divl		%ecx					
	movl		%eax,			%ebx	
	movl	  8(%esp),			%eax	
	divl		%ecx					
	movl		%edx,			%eax	
	popl		%ebx					
	xorl		%edx,			%edx	
	retl
