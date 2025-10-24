	.file	"main.c"
	.text
.lcomm display,8,8
.lcomm indev,8,8
.lcomm buf1,96000,32
.lcomm buf2,96000,32
.lcomm mouse_data,104,32
.lcomm g_hwnd,8,8
.lcomm g_hdc,8,8
.lcomm g_hglrc,8,8
.lcomm g_tex,4,4
.lcomm g_framebuf,1920000,32
	.data
	.align 4
g_width:
	.long	800
	.align 4
g_height:
	.long	600
	.text
	.def	display_flush_cb;	.scl	3;	.type	32;	.endef
	.seh_proc	display_flush_cb
display_flush_cb:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$112, %rsp
	.seh_stackalloc	112
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	movq	%r8, 32(%rbp)
	movq	24(%rbp), %rax
	movl	8(%rax), %edx
	movq	24(%rbp), %rax
	movl	(%rax), %eax
	subl	%eax, %edx
	leal	1(%rdx), %eax
	movl	%eax, -12(%rbp)
	movq	24(%rbp), %rax
	movl	12(%rax), %edx
	movq	24(%rbp), %rax
	movl	4(%rax), %eax
	subl	%eax, %edx
	leal	1(%rdx), %eax
	movl	%eax, -16(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L2
.L5:
	movl	$0, -8(%rbp)
	jmp	.L3
.L4:
	movl	-4(%rbp), %eax
	imull	-12(%rbp), %eax
	movl	-8(%rbp), %edx
	addl	%edx, %eax
	cltq
	leaq	(%rax,%rax), %rdx
	movq	32(%rbp), %rax
	addq	%rdx, %rax
	movzwl	(%rax), %eax
	movw	%ax, -18(%rbp)
	movzwl	-18(%rbp), %eax
	shrw	$11, %ax
	sall	$3, %eax
	movb	%al, -19(%rbp)
	movzwl	-18(%rbp), %eax
	shrw	$5, %ax
	sall	$2, %eax
	movb	%al, -20(%rbp)
	movzwl	-18(%rbp), %eax
	sall	$3, %eax
	movb	%al, -21(%rbp)
	movq	24(%rbp), %rax
	movl	(%rax), %edx
	movl	-8(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -28(%rbp)
	movq	24(%rbp), %rax
	movl	4(%rax), %edx
	movl	-4(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -32(%rbp)
	movzbl	-19(%rbp), %eax
	sall	$16, %eax
	movl	%eax, %edx
	movzbl	-20(%rbp), %eax
	sall	$8, %eax
	orl	%eax, %edx
	movzbl	-21(%rbp), %eax
	orl	%eax, %edx
	movl	g_width(%rip), %eax
	imull	-32(%rbp), %eax
	movl	-28(%rbp), %ecx
	addl	%ecx, %eax
	movl	%edx, %ecx
	orl	$-16777216, %ecx
	cltq
	leaq	0(,%rax,4), %rdx
	leaq	g_framebuf(%rip), %rax
	movl	%ecx, (%rdx,%rax)
	addl	$1, -8(%rbp)
.L3:
	movl	-8(%rbp), %eax
	cmpl	-12(%rbp), %eax
	jl	.L4
	addl	$1, -4(%rbp)
.L2:
	movl	-4(%rbp), %eax
	cmpl	-16(%rbp), %eax
	jl	.L5
	movq	g_hglrc(%rip), %rax
	testq	%rax, %rax
	je	.L6
	movl	g_tex(%rip), %eax
	testl	%eax, %eax
	je	.L6
	movq	g_hglrc(%rip), %rdx
	movq	g_hdc(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_wglMakeCurrent(%rip), %rax
	call	*%rax
	movl	g_tex(%rip), %eax
	movl	%eax, %edx
	movl	$3553, %ecx
	movq	__imp_glBindTexture(%rip), %rax
	call	*%rax
	movl	$1, %edx
	movl	$3317, %ecx
	movq	__imp_glPixelStorei(%rip), %rax
	call	*%rax
	movl	g_width(%rip), %eax
	movl	%eax, %edx
	movl	$3314, %ecx
	movq	__imp_glPixelStorei(%rip), %rax
	call	*%rax
	movq	24(%rbp), %rax
	movl	4(%rax), %edx
	movl	g_width(%rip), %eax
	imull	%edx, %eax
	movq	24(%rbp), %rdx
	movl	(%rdx), %edx
	addl	%edx, %eax
	cltq
	leaq	0(,%rax,4), %rdx
	leaq	g_framebuf(%rip), %rax
	addq	%rdx, %rax
	movq	24(%rbp), %rdx
	movl	4(%rdx), %ecx
	movq	24(%rbp), %rdx
	movl	(%rdx), %edx
	movq	%rax, 64(%rsp)
	movl	$5121, 56(%rsp)
	movl	$6408, 48(%rsp)
	movl	-16(%rbp), %eax
	movl	%eax, 40(%rsp)
	movl	-12(%rbp), %eax
	movl	%eax, 32(%rsp)
	movl	%ecx, %r9d
	movl	%edx, %r8d
	movl	$0, %edx
	movl	$3553, %ecx
	movq	__imp_glTexSubImage2D(%rip), %rax
	call	*%rax
	movl	$0, %edx
	movl	$3314, %ecx
	movq	__imp_glPixelStorei(%rip), %rax
	call	*%rax
	movl	g_height(%rip), %edx
	movl	g_width(%rip), %eax
	movl	%edx, %r9d
	movl	%eax, %r8d
	movl	$0, %edx
	movl	$0, %ecx
	movq	__imp_glViewport(%rip), %rax
	call	*%rax
	movl	$16384, %ecx
	movq	__imp_glClear(%rip), %rax
	call	*%rax
	movl	$5888, %ecx
	movq	__imp_glMatrixMode(%rip), %rax
	call	*%rax
	movq	__imp_glLoadIdentity(%rip), %rax
	call	*%rax
	movl	$3553, %ecx
	movq	__imp_glEnable(%rip), %rax
	call	*%rax
	movl	g_tex(%rip), %eax
	movl	%eax, %edx
	movl	$3553, %ecx
	movq	__imp_glBindTexture(%rip), %rax
	call	*%rax
	movl	$7, %ecx
	movq	__imp_glBegin(%rip), %rax
	call	*%rax
	pxor	%xmm1, %xmm1
	movl	.LC0(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glTexCoord2f(%rip), %rax
	call	*%rax
	movss	.LC1(%rip), %xmm1
	movl	.LC2(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glVertex2f(%rip), %rax
	call	*%rax
	pxor	%xmm1, %xmm1
	movl	.LC1(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glTexCoord2f(%rip), %rax
	call	*%rax
	movss	.LC1(%rip), %xmm1
	movl	.LC1(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glVertex2f(%rip), %rax
	call	*%rax
	movss	.LC1(%rip), %xmm1
	movl	.LC1(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glTexCoord2f(%rip), %rax
	call	*%rax
	movss	.LC2(%rip), %xmm1
	movl	.LC1(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glVertex2f(%rip), %rax
	call	*%rax
	movss	.LC1(%rip), %xmm1
	movl	.LC0(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glTexCoord2f(%rip), %rax
	call	*%rax
	movss	.LC2(%rip), %xmm1
	movl	.LC2(%rip), %eax
	movd	%eax, %xmm0
	movq	__imp_glVertex2f(%rip), %rax
	call	*%rax
	movq	__imp_glEnd(%rip), %rax
	call	*%rax
	movq	g_hdc(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_SwapBuffers(%rip), %rax
	call	*%rax
.L6:
	movq	16(%rbp), %rax
	movq	%rax, %rcx
	call	lv_display_flush_ready
	nop
	addq	$112, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC3:
	.ascii "\342\234\223 LVGL initialized\0"
	.align 8
.LC4:
	.ascii "\342\234\223 Initial UI refresh completed\0"
	.align 8
.LC5:
	.ascii "\12*** UI CREATED WITH MOUSE INPUT SIMULATION ***\0"
	.align 8
.LC6:
	.ascii "Watch the console for UI rendering activity!\0"
.LC7:
	.ascii "Commands:\0"
	.align 8
.LC8:
	.ascii "  '1' - Simulate click on button (center: 400,300)\0"
	.align 8
.LC9:
	.ascii "  '2' - Simulate click on red rectangle (200,150)\0"
.LC10:
	.ascii "  'q' - Quit\12\0"
	.align 8
.LC11:
	.ascii "Entering Windows message loop (OpenGL). Close window to exit.\0"
	.text
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$112, %rsp
	.seh_stackalloc	112
	.seh_endprologue
	call	__main
	call	lv_init
	leaq	.LC3(%rip), %rax
	movq	%rax, %rcx
	call	puts
	call	hal_init
	call	ui_init
	call	lv_screen_active
	movq	%rax, %rcx
	call	lv_obj_invalidate
	movq	display(%rip), %rax
	movq	%rax, %rcx
	call	lv_refr_now
	leaq	.LC4(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	.LC5(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	.LC6(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	.LC7(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	.LC8(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	.LC9(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	.LC10(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	.LC11(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movb	$1, -1(%rbp)
	jmp	.L8
.L12:
	movl	-56(%rbp), %eax
	cmpl	$18, %eax
	jne	.L10
	movb	$0, -1(%rbp)
	jmp	.L11
.L10:
	leaq	-64(%rbp), %rax
	movq	%rax, %rcx
	movq	__imp_TranslateMessage(%rip), %rax
	call	*%rax
	leaq	-64(%rbp), %rax
	movq	%rax, %rcx
	movq	__imp_DispatchMessageA(%rip), %rax
	call	*%rax
.L9:
	leaq	-64(%rbp), %rax
	movl	$1, 32(%rsp)
	movl	$0, %r9d
	movl	$0, %r8d
	movl	$0, %edx
	movq	%rax, %rcx
	movq	__imp_PeekMessageA(%rip), %rax
	call	*%rax
	testl	%eax, %eax
	jne	.L12
.L11:
	call	lv_timer_handler
	movl	$5, %ecx
	movq	__imp_Sleep(%rip), %rax
	call	*%rax
.L8:
	cmpb	$0, -1(%rbp)
	jne	.L9
	call	cleanup_opengl
	movl	$0, %eax
	addq	$112, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC12:
	.ascii "w\0"
.LC13:
	.ascii "CONOUT$\0"
	.align 8
.LC14:
	.ascii "=== LVGL OpenGL/Console Demo ===\0"
	.align 8
.LC15:
	.ascii "ERROR: Failed to initialize OpenGL window. Falling back to console output.\0"
	.align 8
.LC16:
	.ascii "ERROR: Failed to create display\0"
	.align 8
.LC17:
	.ascii "\342\234\223 Display and input device initialized\0"
	.text
	.def	hal_init;	.scl	3;	.type	32;	.endef
	.seh_proc	hal_init
hal_init:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$96, %rsp
	.seh_stackalloc	96
	.seh_endprologue
	movq	__imp_AllocConsole(%rip), %rax
	call	*%rax
	movl	$1, %ecx
	movq	__imp___acrt_iob_func(%rip), %rax
	call	*%rax
	movq	%rax, %rcx
	leaq	.LC12(%rip), %r8
	leaq	.LC13(%rip), %rdx
	leaq	-8(%rbp), %rax
	movq	%rcx, %r9
	movq	%rax, %rcx
	movq	__imp_freopen_s(%rip), %rax
	call	*%rax
	leaq	.LC14(%rip), %rax
	movq	%rax, %rcx
	call	puts
	call	init_opengl_window
	xorl	$1, %eax
	testb	%al, %al
	je	.L16
	leaq	.LC15(%rip), %rax
	movq	%rax, %rcx
	call	puts
	jmp	.L17
.L16:
	movq	g_hglrc(%rip), %rdx
	movq	g_hdc(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_wglMakeCurrent(%rip), %rax
	call	*%rax
	leaq	g_tex(%rip), %rax
	movq	%rax, %rdx
	movl	$1, %ecx
	movq	__imp_glGenTextures(%rip), %rax
	call	*%rax
	movl	g_tex(%rip), %eax
	movl	%eax, %edx
	movl	$3553, %ecx
	movq	__imp_glBindTexture(%rip), %rax
	call	*%rax
	movl	$9728, %r8d
	movl	$10241, %edx
	movl	$3553, %ecx
	movq	__imp_glTexParameteri(%rip), %rax
	call	*%rax
	movl	$9728, %r8d
	movl	$10240, %edx
	movl	$3553, %ecx
	movq	__imp_glTexParameteri(%rip), %rax
	call	*%rax
	movl	g_height(%rip), %eax
	movl	g_width(%rip), %ecx
	leaq	g_framebuf(%rip), %rdx
	movq	%rdx, 64(%rsp)
	movl	$5121, 56(%rsp)
	movl	$6408, 48(%rsp)
	movl	$0, 40(%rsp)
	movl	%eax, 32(%rsp)
	movl	%ecx, %r9d
	movl	$6408, %r8d
	movl	$0, %edx
	movl	$3553, %ecx
	movq	__imp_glTexImage2D(%rip), %rax
	call	*%rax
.L17:
	movl	$600, %edx
	movl	$800, %ecx
	call	lv_display_create
	movq	%rax, display(%rip)
	movq	display(%rip), %rax
	testq	%rax, %rax
	jne	.L18
	leaq	.LC16(%rip), %rax
	movq	%rax, %rcx
	call	puts
	jmp	.L15
.L18:
	movq	display(%rip), %rax
	leaq	buf2(%rip), %rcx
	leaq	buf1(%rip), %rdx
	movl	$0, 32(%rsp)
	movl	$96000, %r9d
	movq	%rcx, %r8
	movq	%rax, %rcx
	call	lv_display_set_buffers
	movq	display(%rip), %rax
	leaq	display_flush_cb(%rip), %rdx
	movq	%rax, %rcx
	call	lv_display_set_flush_cb
	movl	$0, 76+mouse_data(%rip)
	movl	$0, 80+mouse_data(%rip)
	movl	$0, 72+mouse_data(%rip)
	call	lv_indev_create
	movq	%rax, indev(%rip)
	movq	indev(%rip), %rax
	movl	$1, %edx
	movq	%rax, %rcx
	call	lv_indev_set_type
	movq	indev(%rip), %rax
	leaq	mouse_read(%rip), %rdx
	movq	%rax, %rcx
	call	lv_indev_set_read_cb
	movq	display(%rip), %rdx
	movq	indev(%rip), %rax
	movq	%rax, %rcx
	call	lv_indev_set_display
	leaq	.LC17(%rip), %rax
	movq	%rax, %rcx
	call	puts
	nop
.L15:
	addq	$96, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC18:
	.ascii "Creating UI elements...\0"
	.align 8
.LC19:
	.ascii "\342\234\223 Active screen obtained: %p\12\0"
.LC20:
	.ascii "\342\234\223 Screen background set\0"
	.align 8
.LC21:
	.ascii "\342\234\223 Button created and centered\0"
.LC22:
	.ascii "\342\234\223 Red rectangle created\0"
.LC23:
	.ascii "Click Me!\0"
	.align 8
.LC24:
	.ascii "LVGL Windows Demo with Input Simulation\0"
	.align 8
.LC25:
	.ascii "Press '1' to click button, '2' for rectangle, 'q' to quit\0"
	.text
	.def	ui_init;	.scl	3;	.type	32;	.endef
	.seh_proc	ui_init
ui_init:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$144, %rsp
	.seh_stackalloc	144
	.seh_endprologue
	leaq	.LC18(%rip), %rax
	movq	%rax, %rcx
	call	puts
	call	lv_screen_active
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rdx
	leaq	.LC19(%rip), %rax
	movq	%rax, %rcx
	call	printf
	leaq	-74(%rbp), %rax
	movl	$15263976, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-74(%rbp), %eax
	movw	%ax, -96(%rbp)
	movzbl	-72(%rbp), %eax
	movb	%al, -94(%rbp)
	leaq	-96(%rbp), %rax
	movq	-8(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_bg_color
	movq	-8(%rbp), %rax
	movl	$0, %r8d
	movl	$255, %edx
	movq	%rax, %rcx
	call	lv_obj_set_style_bg_opa
	leaq	.LC20(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_button_create
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movl	$60, %r8d
	movl	$150, %edx
	movq	%rax, %rcx
	call	lv_obj_set_size
	movq	-16(%rbp), %rax
	movq	%rax, %rcx
	call	lv_obj_center
	leaq	btn_event_cb(%rip), %rdx
	movq	-16(%rbp), %rax
	movl	$0, %r9d
	movl	$10, %r8d
	movq	%rax, %rcx
	call	lv_obj_add_event_cb
	leaq	.LC21(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_obj_create
	movq	%rax, -24(%rbp)
	movq	-24(%rbp), %rax
	movl	$100, %r8d
	movl	$200, %edx
	movq	%rax, %rcx
	call	lv_obj_set_size
	movq	-24(%rbp), %rax
	movl	$100, %r8d
	movl	$100, %edx
	movq	%rax, %rcx
	call	lv_obj_set_pos
	leaq	-71(%rbp), %rax
	movl	$16711680, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-71(%rbp), %eax
	movw	%ax, -96(%rbp)
	movzbl	-69(%rbp), %eax
	movb	%al, -94(%rbp)
	leaq	-96(%rbp), %rax
	movq	-24(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_bg_color
	movq	-24(%rbp), %rax
	movl	$0, %r8d
	movl	$255, %edx
	movq	%rax, %rcx
	call	lv_obj_set_style_bg_opa
	leaq	.LC22(%rip), %rax
	movq	%rax, %rcx
	call	puts
	leaq	-68(%rbp), %rax
	movl	$2201331, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-68(%rbp), %eax
	movw	%ax, -96(%rbp)
	movzbl	-66(%rbp), %eax
	movb	%al, -94(%rbp)
	leaq	-96(%rbp), %rax
	movq	-16(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_bg_color
	movq	-16(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_create
	movq	%rax, -32(%rbp)
	leaq	.LC23(%rip), %rdx
	movq	-32(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_set_text
	movq	-32(%rbp), %rax
	movq	%rax, %rcx
	call	lv_obj_center
	leaq	-65(%rbp), %rax
	movl	$16777215, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-65(%rbp), %eax
	movw	%ax, -96(%rbp)
	movzbl	-63(%rbp), %eax
	movb	%al, -94(%rbp)
	leaq	-96(%rbp), %rax
	movq	-32(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_text_color
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_slider_create
	movq	%rax, -40(%rbp)
	movq	-40(%rbp), %rax
	movl	$30, %r8d
	movl	$250, %edx
	movq	%rax, %rcx
	call	lv_obj_set_size
	movq	-16(%rbp), %rdx
	movq	-40(%rbp), %rax
	movl	$40, 32(%rsp)
	movl	$0, %r9d
	movl	$14, %r8d
	movq	%rax, %rcx
	call	lv_obj_align_to
	movq	-40(%rbp), %rax
	movl	$0, %r8d
	movl	$50, %edx
	movq	%rax, %rcx
	call	lv_slider_set_value
	leaq	slider_event_cb(%rip), %rdx
	movq	-40(%rbp), %rax
	movl	$0, %r9d
	movl	$35, %r8d
	movq	%rax, %rcx
	call	lv_obj_add_event_cb
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_create
	movq	%rax, -48(%rbp)
	leaq	.LC24(%rip), %rdx
	movq	-48(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_set_text
	movq	-16(%rbp), %rdx
	movq	-48(%rbp), %rax
	movl	$-40, 32(%rsp)
	movl	$0, %r9d
	movl	$11, %r8d
	movq	%rax, %rcx
	call	lv_obj_align_to
	leaq	-62(%rbp), %rax
	movl	$3355443, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-62(%rbp), %eax
	movw	%ax, -96(%rbp)
	movzbl	-60(%rbp), %eax
	movb	%al, -94(%rbp)
	leaq	-96(%rbp), %rax
	movq	-48(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_text_color
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_create
	movq	%rax, -56(%rbp)
	leaq	.LC25(%rip), %rdx
	movq	-56(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_set_text
	movq	-40(%rbp), %rdx
	movq	-56(%rbp), %rax
	movl	$30, 32(%rsp)
	movl	$0, %r9d
	movl	$14, %r8d
	movq	%rax, %rcx
	call	lv_obj_align_to
	leaq	-59(%rbp), %rax
	movl	$6710886, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-59(%rbp), %eax
	movw	%ax, -96(%rbp)
	movzbl	-57(%rbp), %eax
	movb	%al, -94(%rbp)
	leaq	-96(%rbp), %rax
	movq	-56(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_text_color
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_obj_invalidate
	movq	display(%rip), %rax
	movq	%rax, %rcx
	call	lv_refr_now
	nop
	addq	$144, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC26:
	.ascii "Clicked!\0"
	.align 8
.LC27:
	.ascii "*** BUTTON CLICKED - UI INTERACTION WORKS! ***\0"
.LC28:
	.ascii "*** BUTTON RESET ***\0"
	.text
	.def	btn_event_cb;	.scl	3;	.type	32;	.endef
	.seh_proc	btn_event_cb
btn_event_cb:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$80, %rsp
	.seh_stackalloc	80
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rax
	movq	%rax, %rcx
	call	lv_event_get_target
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	$0, %edx
	movq	%rax, %rcx
	call	lv_obj_get_child
	movq	%rax, -16(%rbp)
	movzbl	clicked.0(%rip), %eax
	movzbl	%al, %eax
	testl	%eax, %eax
	setne	%al
	xorl	$1, %eax
	movzbl	%al, %eax
	andl	$1, %eax
	movb	%al, clicked.0(%rip)
	movzbl	clicked.0(%rip), %eax
	testb	%al, %al
	je	.L22
	leaq	.LC26(%rip), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_set_text
	leaq	-22(%rbp), %rax
	movl	$5025616, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-22(%rbp), %eax
	movw	%ax, -48(%rbp)
	movzbl	-20(%rbp), %eax
	movb	%al, -46(%rbp)
	leaq	-48(%rbp), %rax
	movq	-8(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_bg_color
	leaq	.LC27(%rip), %rax
	movq	%rax, %rcx
	call	puts
	jmp	.L23
.L22:
	leaq	.LC23(%rip), %rdx
	movq	-16(%rbp), %rax
	movq	%rax, %rcx
	call	lv_label_set_text
	leaq	-19(%rbp), %rax
	movl	$2201331, %edx
	movq	%rax, %rcx
	call	lv_color_hex
	movzwl	-19(%rbp), %eax
	movw	%ax, -48(%rbp)
	movzbl	-17(%rbp), %eax
	movb	%al, -46(%rbp)
	leaq	-48(%rbp), %rax
	movq	-8(%rbp), %rcx
	movl	$0, %r8d
	movq	%rax, %rdx
	call	lv_obj_set_style_bg_color
	leaq	.LC28(%rip), %rax
	movq	%rax, %rcx
	call	puts
.L23:
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_obj_invalidate
	nop
	addq	$80, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC29:
	.ascii "Slider changed: %d\12\0"
	.text
	.def	slider_event_cb;	.scl	3;	.type	32;	.endef
	.seh_proc	slider_event_cb
slider_event_cb:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rax
	movq	%rax, %rcx
	call	lv_event_get_target
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_slider_get_value
	movl	%eax, -12(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_obj_invalidate
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	lv_obj_get_parent
	movq	%rax, %rcx
	call	lv_obj_invalidate
	movl	-12(%rbp), %edx
	leaq	.LC29(%rip), %rax
	movq	%rax, %rcx
	call	printf
	nop
	addq	$48, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.def	mouse_read;	.scl	3;	.type	32;	.endef
	.seh_proc	mouse_read
mouse_read:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	movl	76+mouse_data(%rip), %edx
	movq	24(%rbp), %rax
	movl	%edx, 76(%rax)
	movl	80+mouse_data(%rip), %edx
	movq	24(%rbp), %rax
	movl	%edx, 80(%rax)
	movl	72+mouse_data(%rip), %edx
	movq	24(%rbp), %rax
	movl	%edx, 72(%rax)
	nop
	popq	%rbp
	ret
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC30:
	.ascii "Mouse click simulated at (%d, %d)\12\0"
	.text
	.def	simulate_mouse_click;	.scl	3;	.type	32;	.endef
	.seh_proc	simulate_mouse_click
simulate_mouse_click:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movl	%ecx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movl	16(%rbp), %eax
	movl	%eax, 76+mouse_data(%rip)
	movl	24(%rbp), %eax
	movl	%eax, 80+mouse_data(%rip)
	movl	$1, 72+mouse_data(%rip)
	call	lv_timer_handler
	movl	$50, %ecx
	movq	__imp_Sleep(%rip), %rax
	call	*%rax
	movl	$0, 72+mouse_data(%rip)
	call	lv_timer_handler
	movl	24(%rbp), %ecx
	movl	16(%rbp), %edx
	leaq	.LC30(%rip), %rax
	movl	%ecx, %r8d
	movq	%rax, %rcx
	call	printf
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.def	WndProc;	.scl	3;	.type	32;	.endef
	.seh_proc	WndProc
WndProc:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$64, %rsp
	.seh_stackalloc	64
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movl	%edx, 24(%rbp)
	movq	%r8, 32(%rbp)
	movq	%r9, 40(%rbp)
	cmpl	$514, 24(%rbp)
	je	.L28
	cmpl	$514, 24(%rbp)
	ja	.L29
	cmpl	$513, 24(%rbp)
	je	.L30
	cmpl	$513, 24(%rbp)
	ja	.L29
	cmpl	$16, 24(%rbp)
	je	.L31
	cmpl	$512, 24(%rbp)
	je	.L32
	jmp	.L29
.L31:
	movl	$0, %ecx
	movq	__imp_PostQuitMessage(%rip), %rax
	call	*%rax
	movl	$0, %eax
	jmp	.L33
.L30:
	movq	40(%rbp), %rax
	cwtl
	movl	%eax, -12(%rbp)
	movq	40(%rbp), %rax
	shrq	$16, %rax
	cwtl
	movl	%eax, -16(%rbp)
	movl	-12(%rbp), %eax
	movl	%eax, 76+mouse_data(%rip)
	movl	-16(%rbp), %eax
	movl	%eax, 80+mouse_data(%rip)
	movl	$1, 72+mouse_data(%rip)
	movl	$0, %eax
	jmp	.L33
.L28:
	movq	40(%rbp), %rax
	cwtl
	movl	%eax, -4(%rbp)
	movq	40(%rbp), %rax
	shrq	$16, %rax
	cwtl
	movl	%eax, -8(%rbp)
	movl	-4(%rbp), %eax
	movl	%eax, 76+mouse_data(%rip)
	movl	-8(%rbp), %eax
	movl	%eax, 80+mouse_data(%rip)
	movl	$0, 72+mouse_data(%rip)
	movl	$0, %eax
	jmp	.L33
.L32:
	movq	40(%rbp), %rax
	cwtl
	movl	%eax, -20(%rbp)
	movq	40(%rbp), %rax
	shrq	$16, %rax
	cwtl
	movl	%eax, -24(%rbp)
	movl	-20(%rbp), %eax
	movl	%eax, 76+mouse_data(%rip)
	movl	-24(%rbp), %eax
	movl	%eax, 80+mouse_data(%rip)
	movl	$0, %eax
	jmp	.L33
.L29:
	movq	40(%rbp), %r8
	movq	32(%rbp), %rcx
	movl	24(%rbp), %edx
	movq	16(%rbp), %rax
	movq	%r8, %r9
	movq	%rcx, %r8
	movq	%rax, %rcx
	movq	__imp_DefWindowProcA(%rip), %rax
	call	*%rax
.L33:
	addq	$64, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section .rdata,"dr"
.LC31:
	.ascii "lvgl_opengl_window\0"
.LC32:
	.ascii "RegisterClass failed\0"
.LC33:
	.ascii "LVGL OpenGL\0"
.LC34:
	.ascii "CreateWindow failed\0"
.LC35:
	.ascii "ChoosePixelFormat failed\0"
.LC36:
	.ascii "SetPixelFormat failed\0"
.LC37:
	.ascii "wglCreateContext failed\0"
.LC38:
	.ascii "wglMakeCurrent failed\0"
	.text
	.def	init_opengl_window;	.scl	3;	.type	32;	.endef
	.seh_proc	init_opengl_window
init_opengl_window:
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	subq	$248, %rsp
	.seh_stackalloc	248
	leaq	240(%rsp), %rbp
	.seh_setframe	%rbp, 240
	.seh_endprologue
	movl	$0, %ecx
	movq	__imp_GetModuleHandleA(%rip), %rax
	call	*%rax
	movq	%rax, -16(%rbp)
	leaq	-96(%rbp), %rdx
	movl	$0, %eax
	movl	$9, %ecx
	movq	%rdx, %rdi
	rep stosq
	movl	$32, -96(%rbp)
	leaq	WndProc(%rip), %rax
	movq	%rax, -88(%rbp)
	movq	-16(%rbp), %rax
	movq	%rax, -72(%rbp)
	movl	$32512, %edx
	movl	$0, %ecx
	movq	__imp_LoadCursorA(%rip), %rax
	call	*%rax
	movq	%rax, -56(%rbp)
	leaq	.LC31(%rip), %rax
	movq	%rax, -32(%rbp)
	leaq	-96(%rbp), %rax
	movq	%rax, %rcx
	movq	__imp_RegisterClassA(%rip), %rax
	call	*%rax
	testw	%ax, %ax
	jne	.L35
	leaq	.LC32(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movl	$0, %eax
	jmp	.L44
.L35:
	movl	g_height(%rip), %eax
	leal	39(%rax), %ecx
	movl	g_width(%rip), %eax
	leal	16(%rax), %edx
	movq	-32(%rbp), %rax
	leaq	.LC33(%rip), %r10
	movq	$0, 88(%rsp)
	movq	-16(%rbp), %r8
	movq	%r8, 80(%rsp)
	movq	$0, 72(%rsp)
	movq	$0, 64(%rsp)
	movl	%ecx, 56(%rsp)
	movl	%edx, 48(%rsp)
	movl	$-2147483648, 40(%rsp)
	movl	$-2147483648, 32(%rsp)
	movl	$13565952, %r9d
	movq	%r10, %r8
	movq	%rax, %rdx
	movl	$0, %ecx
	movq	__imp_CreateWindowExA(%rip), %rax
	call	*%rax
	movq	%rax, g_hwnd(%rip)
	movq	g_hwnd(%rip), %rax
	testq	%rax, %rax
	jne	.L37
	leaq	.LC34(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movl	$0, %eax
	jmp	.L44
.L37:
	movq	g_hwnd(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_GetDC(%rip), %rax
	call	*%rax
	movq	%rax, g_hdc(%rip)
	movq	$0, -144(%rbp)
	movq	$0, -136(%rbp)
	movq	$0, -128(%rbp)
	movq	$0, -120(%rbp)
	movq	$0, -112(%rbp)
	movw	$40, -144(%rbp)
	movw	$1, -142(%rbp)
	movl	$37, -140(%rbp)
	movb	$0, -136(%rbp)
	movb	$32, -135(%rbp)
	movb	$8, -128(%rbp)
	movq	g_hdc(%rip), %rcx
	leaq	-144(%rbp), %rax
	movq	%rax, %rdx
	movq	__imp_ChoosePixelFormat(%rip), %rax
	call	*%rax
	movl	%eax, -20(%rbp)
	cmpl	$0, -20(%rbp)
	jne	.L38
	leaq	.LC35(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movl	$0, %eax
	jmp	.L44
.L38:
	movq	g_hdc(%rip), %rax
	leaq	-144(%rbp), %rcx
	movl	-20(%rbp), %edx
	movq	%rcx, %r8
	movq	%rax, %rcx
	movq	__imp_SetPixelFormat(%rip), %rax
	call	*%rax
	testl	%eax, %eax
	jne	.L39
	leaq	.LC36(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movl	$0, %eax
	jmp	.L44
.L39:
	movq	g_hdc(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_wglCreateContext(%rip), %rax
	call	*%rax
	movq	%rax, g_hglrc(%rip)
	movq	g_hglrc(%rip), %rax
	testq	%rax, %rax
	jne	.L40
	leaq	.LC37(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movl	$0, %eax
	jmp	.L44
.L40:
	movq	g_hglrc(%rip), %rdx
	movq	g_hdc(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_wglMakeCurrent(%rip), %rax
	call	*%rax
	testl	%eax, %eax
	jne	.L41
	leaq	.LC38(%rip), %rax
	movq	%rax, %rcx
	call	puts
	movl	$0, %eax
	jmp	.L44
.L41:
	movl	$0, -4(%rbp)
	jmp	.L42
.L43:
	movl	-4(%rbp), %eax
	cltq
	leaq	0(,%rax,4), %rdx
	leaq	g_framebuf(%rip), %rax
	movl	$-1, (%rdx,%rax)
	addl	$1, -4(%rbp)
.L42:
	movl	g_width(%rip), %edx
	movl	g_height(%rip), %eax
	imull	%edx, %eax
	cmpl	%eax, -4(%rbp)
	jl	.L43
	movq	g_hwnd(%rip), %rax
	movl	$5, %edx
	movq	%rax, %rcx
	movq	__imp_ShowWindow(%rip), %rax
	call	*%rax
	movq	g_hwnd(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_UpdateWindow(%rip), %rax
	call	*%rax
	movl	$1, %eax
.L44:
	addq	$248, %rsp
	popq	%rdi
	popq	%rbp
	ret
	.seh_endproc
	.def	cleanup_opengl;	.scl	3;	.type	32;	.endef
	.seh_proc	cleanup_opengl
cleanup_opengl:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	g_hglrc(%rip), %rax
	testq	%rax, %rax
	je	.L46
	movl	$0, %edx
	movl	$0, %ecx
	movq	__imp_wglMakeCurrent(%rip), %rax
	call	*%rax
	movq	g_hglrc(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_wglDeleteContext(%rip), %rax
	call	*%rax
	movq	$0, g_hglrc(%rip)
.L46:
	movq	g_hdc(%rip), %rax
	testq	%rax, %rax
	je	.L47
	movq	g_hwnd(%rip), %rax
	testq	%rax, %rax
	je	.L47
	movq	g_hdc(%rip), %rdx
	movq	g_hwnd(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_ReleaseDC(%rip), %rax
	call	*%rax
	movq	$0, g_hdc(%rip)
.L47:
	movq	g_hwnd(%rip), %rax
	testq	%rax, %rax
	je	.L49
	movq	g_hwnd(%rip), %rax
	movq	%rax, %rcx
	movq	__imp_DestroyWindow(%rip), %rax
	call	*%rax
	movq	$0, g_hwnd(%rip)
.L49:
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
.lcomm clicked.0,1,1
	.section .rdata,"dr"
	.align 4
.LC0:
	.long	0
	.align 4
.LC1:
	.long	1065353216
	.align 4
.LC2:
	.long	-1082130432
	.def	__main;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (x86_64-win32-seh-rev0, Built by MinGW-Builds project) 15.1.0"
	.def	lv_display_flush_ready;	.scl	2;	.type	32;	.endef
	.def	lv_init;	.scl	2;	.type	32;	.endef
	.def	puts;	.scl	2;	.type	32;	.endef
	.def	lv_screen_active;	.scl	2;	.type	32;	.endef
	.def	lv_obj_invalidate;	.scl	2;	.type	32;	.endef
	.def	lv_refr_now;	.scl	2;	.type	32;	.endef
	.def	lv_timer_handler;	.scl	2;	.type	32;	.endef
	.def	lv_display_create;	.scl	2;	.type	32;	.endef
	.def	lv_display_set_buffers;	.scl	2;	.type	32;	.endef
	.def	lv_display_set_flush_cb;	.scl	2;	.type	32;	.endef
	.def	lv_indev_create;	.scl	2;	.type	32;	.endef
	.def	lv_indev_set_type;	.scl	2;	.type	32;	.endef
	.def	lv_indev_set_read_cb;	.scl	2;	.type	32;	.endef
	.def	lv_indev_set_display;	.scl	2;	.type	32;	.endef
	.def	printf;	.scl	2;	.type	32;	.endef
	.def	lv_color_hex;	.scl	2;	.type	32;	.endef
	.def	lv_obj_set_style_bg_color;	.scl	2;	.type	32;	.endef
	.def	lv_obj_set_style_bg_opa;	.scl	2;	.type	32;	.endef
	.def	lv_button_create;	.scl	2;	.type	32;	.endef
	.def	lv_obj_set_size;	.scl	2;	.type	32;	.endef
	.def	lv_obj_center;	.scl	2;	.type	32;	.endef
	.def	lv_obj_add_event_cb;	.scl	2;	.type	32;	.endef
	.def	lv_obj_create;	.scl	2;	.type	32;	.endef
	.def	lv_obj_set_pos;	.scl	2;	.type	32;	.endef
	.def	lv_label_create;	.scl	2;	.type	32;	.endef
	.def	lv_label_set_text;	.scl	2;	.type	32;	.endef
	.def	lv_obj_set_style_text_color;	.scl	2;	.type	32;	.endef
	.def	lv_slider_create;	.scl	2;	.type	32;	.endef
	.def	lv_obj_align_to;	.scl	2;	.type	32;	.endef
	.def	lv_slider_set_value;	.scl	2;	.type	32;	.endef
	.def	lv_event_get_target;	.scl	2;	.type	32;	.endef
	.def	lv_obj_get_child;	.scl	2;	.type	32;	.endef
	.def	lv_slider_get_value;	.scl	2;	.type	32;	.endef
	.def	lv_obj_get_parent;	.scl	2;	.type	32;	.endef
