---
layout: default
---

<pre>
[ 0.000000] <a href="lessons/06_preprocessor_truth.md">PREPROCESSOR</a> : #include &lt;sys/socket.h&gt; :: AF_INET -> 2
[ 0.000001] <a href="lessons/06_preprocessor_truth.md">PREPROCESSOR</a> : #include &lt;sys/socket.h&gt; :: SOCK_STREAM -> 1
[ 0.000002] <a href="lessons/07_register_truth.md">USER_ASM    </a> : mov $2, %rdi  (family)
[ 0.000003] <a href="lessons/07_register_truth.md">USER_ASM    </a> : mov $1, %rsi  (type)
[ 0.000004] <a href="lessons/07_register_truth.md">USER_ASM    </a> : mov $0, %rdx  (protocol)
[ 0.000005] <a href="lessons/08_syscall_trap.md">HARDWARE    </a> : MSR_LSTAR = 0xffffffff97800080
[ 0.000006] <a href="lessons/08_syscall_trap.md">TRAP        </a> : mov $41, %rax (__NR_socket)
[ 0.000007] <a href="lessons/08_syscall_trap.md">TRAP        </a> : syscall       (jmp MSR_LSTAR)
[ 0.000010] <a href="lessons/05_axiomatic_trace.md">KERNEL      </a> : entry_SYSCALL_64 -> do_syscall_64
[ 0.000011] <a href="lessons/05_axiomatic_trace.md">KERNEL      </a> : __sys_socket(2, 1, 0)
[ 0.000012] <a href="lessons/01_slab_geometry.md">SLAB        </a> : kmem_cache_alloc(sock_inode_cache)
[ 0.000013] <a href="lessons/01_slab_geometry.md">SLAB        </a> : Allocated 832 bytes (requested: 768)
[ 0.000013] <a href="lessons/01_slab_geometry.md">SLAB_DEBUG  </a> : Replica(768) = 768 bytes (Delta 0) (Overhead confirmed unique)
[ 0.000014] <a href="lessons/01_slab_geometry.md">GEOMETRY    </a> : inode  @ offset 128
[ 0.000015] <a href="lessons/01_slab_geometry.md">GEOMETRY    </a> : socket @ offset 0
[ 0.000016] <a href="lessons/03_protocol_collision.md">LOOKUP      </a> : inetsw[1] linked_list walk
[ 0.000017] <a href="lessons/03_protocol_collision.md">LOOKUP      </a> : Found TCP (type=1, protocol=6)
[ 0.000018] <a href="lessons/11_russian_dolls.md">DEPTH       </a> : sk_alloc(TCP)
[ 0.000019] <a href="lessons/11_russian_dolls.md">DEPTH       </a> : Allocated 2360 bytes (struct tcp_sock)
[ 0.000020] <a href="lessons/04_primate_arithmetic.md">RETURN      </a> : VFS returns struct inode *
[ 0.000021] <a href="lessons/04_primate_arithmetic.md">RETURN      </a> : SOCKET_I(inode) -> socket (inode - 128)
[ 0.000022] <a href="lessons/04_primate_arithmetic.md">EXIT        </a> : fd_install(3, file)
[ 0.000022] <a href="lessons/04_primate_arithmetic.md">EXIT_MATH   </a> : current->files->fdt->fd[3] == file (Verified)
[ 0.000023] <a href="lessons/04_primate_arithmetic.md">EXIT        </a> : return 3

[ MANUALS  ]
[ MAN 00   ] <a href="lessons/10_lkml_manifesto.md">THE_LKML_MANIFESTO</a> (Zero Tolerance)
[ MAN 01   ] <a href="lessons/00_llm_preface.md">INTELLIGENT_AGENTS</a> (Usage Guide)
[ MAN 02   ] <a href="lessons/02_mistakes_log.md">MISTAKES_LOG      </a> (Retired Metaphors)
</pre>
