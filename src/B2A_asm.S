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


	.global combine1_asm
	.type combine1_asm, %function
	.align 2
	combine1_asm:
	  push {r4-r5, r14}

	  r1l		.req r0
	  r1h		.req r1
	  r2l		.req r2
	  r2h		.req r3

	 
	  //r1 = r2^r1
	  eor r1l,r2l,r1l
	  eor r1h,r2h,r1h

	  //r1 = r2^r1^x1
	  mov r4, 0
	  mov r5, 0
	  ldrd r4, r5, [sp,#12]
	  eor r1l,r1l,r4
	  eor r1h,r1h,r5

	  //r1 = r2^r1^x1^x2
	  mov r2l,0
	  mov r2h,0
	  ldrd r2l,r2h,[sp,#20]
      eor r1l,r1l,r2l
	  eor r1h,r1h,r2h

	  pop {r4-r5, pc}

	.global combine2_asm
	.type combine2_asm, %function
	.align 2
	combine2_asm:
	  push {r4-r7, r14}

	  r1l		.req r0
	  r1h		.req r1
	  r2l		.req r2
	  r2h		.req r3

	  //load x0
	  ldrd r4,r5,[sp,#36]
	  //y0 = x0 ^ r1
	  eor r4, r1l, r4
	  eor r5, r1h, r5

	  //y0 = x0 ^ r1 ^r2
	  eor r4, r4, r2l
	  eor r5, r5, r2h

	  //load x1
	  ldrd r6,r7,[sp,#20]
	  //y1 = x1 ^ r1
	  eor r6, r6, r1l
	  eor r7, r7,r1h

	  //Psi(y0,y1)
	  //y0 ^ y1
	  eor r1l, r4, r6
	  eor r1h, r5, r7
	  //(y0 ^ y1) - y1
	  subs r1l, r1l, r6
	  sbc r1h, r1h, r7

	  //Psi(y0,y2)
	  mov r6, 0
	  mov r7,0
	  //load x2
	  ldrd r6,r7,[sp,#28]
	  //y2 = x2 ^ r2
	  eor r6, r6, r2l
	  eor r7, r7,r2h

	  //y0 ^ y2
	  eor r2l, r4, r6
	  eor r2h, r5, r7

	  //(y0 ^ y2) -y2
	  subs r2l, r2l, r6
	  sbc r2h, r2h, r7

	  eor r1l, r1l, r4
	  eor r1h, r1h, r5

	  eor r1l, r1l, r2l
	  eor r1h, r1h, r2h
	  
	  pop {r4-r7, pc}

