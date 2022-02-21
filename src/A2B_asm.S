/*
 * MIT License
 *
 * Copyright (c) 2021-2022: Infineon Technologies AG, 85579 Neubiberg, Germany
 * Copyright (c) 2021-2022: Universität der Bundeswehr, 85577 Neubiberg, Germany
 * Authors: Daniel Heinz <daniel.heinz@unibw.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


.syntax unified
.cpu cortex-m4



	.global secure_assign
	.type secure_assign, %function
	.align 2
	secure_assign:
	  push {r14}

	  ptr_out		.req r0
	  ptr_in		.req r1

	  ldr r1, [r1]
	  str r1, [r0]
	  mov r1, 0

	  pop { pc}

	.global SecAND32_asm
	.type SecAND32_asm, %function
	.align 2
	//uint32_t SecAND32_asm(uint32_t x, uint32_t y, uint32_t r);
	SecAND32_asm:
	  push {r14}

	  x .req r0
	  y .req r1
	  r .req r2

	  //x & y
	  and x, x, y
	  mov y, 0

	  //r ^ (x&y)
	  eor x, x, r
	  mov r, 0

      pop {pc}

	.global SecXOR32_asm
	.type SecXOR32_asm, %function
	.align 2
	//uint32_t SecXOR32_asm(uint32_t x, uint32_t y);
	SecXOR32_asm:
	  push {r14}

	  x .req r0
	  y .req r1

	  //x ^ y
	  eor x, x, y
	  mov y, 0

	  pop {pc}

	.global clear_regs
	.type clear_regs, %function
	.align 2
	//uint32_t clear_regs(uint32_t x, uint32_t y);
	clear_regs:
		
		push {r14}

		mov r0, 0
		mov r1, 0
		mov r2, 0
		mov r3, 0

		pop {pc}