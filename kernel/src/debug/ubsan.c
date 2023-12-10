//
// Created by ndraey on 23.09.23.
//

#include "common.h"
#include "debug/ubsan.h"
#include "io/ports.h"

void __ubsan_handle_out_of_bounds(struct type_mismatch_info *type_mismatch,
								  size_t pointer) {
	qemu_err("[%s:%d:%d]: Out of bounds: %x",
			 type_mismatch->location.file,
			 type_mismatch->location.line,
			 type_mismatch->location.column,
			 pointer
	);
}

void __ubsan_handle_pointer_overflow(struct type_mismatch_info *type_mismatch,
									 size_t pointer) {
	qemu_err("Pointer overflow!");
}

void __ubsan_handle_type_mismatch_v1(struct type_mismatch_info *type_mismatch,
									 size_t pointer) {
	if(pointer == 0) {
		qemu_err("[%s:%d:%d]: Null pointer access!",
				 type_mismatch->location.file,
				 type_mismatch->location.line,
				 type_mismatch->location.column);
	} else if(type_mismatch->alignment != 0 &&
			  is_aligned(pointer, type_mismatch->alignment)) {
		qemu_err("[%s:%d:%d]: Misaligned access: %x",
				 type_mismatch->location.file,
				 type_mismatch->location.line,
				 type_mismatch->location.column, pointer);
	} else {
		qemu_err("[%s:%d:%d]: %s address %x with insufficient space for object of type %s",
				 type_mismatch->location.file,
				 type_mismatch->location.line,
				 type_mismatch->location.column,
			 Type_Check_Kinds[type_mismatch->type_check_kind],
			 (void*)pointer,
			 type_mismatch->type->name
		);
	}
}

void __ubsan_handle_mul_overflow(struct type_mismatch_info *type_mismatch,
								 size_t pointer) {
	qemu_err("Multiplication overflow!");
}

void __ubsan_handle_add_overflow(struct type_mismatch_info *type_mismatch,
								 size_t pointer) {
	qemu_err("Addition overflow!");
}

void __ubsan_handle_sub_overflow(struct type_mismatch_info *type_mismatch,
								 size_t pointer) {
	qemu_err("Substraction overflow!");
}

void __ubsan_handle_shift_out_of_bounds(struct type_mismatch_info *type_mismatch,
										size_t pointer) {
	qemu_err("Shift out of bounds!");
}

void __ubsan_handle_divrem_overflow(struct type_mismatch_info *type_mismatch,
									size_t pointer) {
	qemu_err("Division remainder overflow!");
}

void __ubsan_handle_float_cast_overflow(struct type_mismatch_info *type_mismatch,
										size_t pointer) {
	qemu_err("Float cast overflow!");
}

void __ubsan_handle_negate_overflow(struct type_mismatch_info *type_mismatch,
									size_t pointer) {
	qemu_err("Negation overflow!");
}

void __ubsan_handle_vla_bound_not_positive(struct type_mismatch_info *type_mismatch,
										   size_t pointer) {
	qemu_err("VLA bound is not positive!");
}
