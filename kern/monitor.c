// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	cprintf("Stack backtrace:\n");
	
	// get the current ebp
uint32_t ebp;
	asm volatile("movl %%ebp, %0" : "=r" (ebp));

	// get all the six ebp(s) from the current ebp through the stack mechanism
uint32_t* (ebpp[6]);
// use the following strct to log the information of the eip directly
struct Eipdebuginfo info;

	ebpp[0] = (uint32_t*) ebp;
	for(int i = 1; i <= 5; ++i){
		ebpp[i] = (uint32_t*) *ebpp[i-1];
	}

	// print eip, args of the each ebp
	for(int i = 0; i <= 5; ++i){
		cprintf("  %d ebp %08x eip %08x args %08x %08x %08x %08x %08x\n",
			   i, ebpp[i], *(ebpp[i] + 1), *(ebpp[i] + 2), *(ebpp[i] + 3), *(ebpp[i] + 4), *(ebpp[i] + 5),
				*(ebpp[i] + 6));
		if(0 == debuginfo_eip(*(ebpp[i] + 1), &info)){
			cprintf("	%s:%d:%.*s+%d \n", info.eip_file, info.eip_line, 
						info.eip_fn_namelen, info.eip_fn_name,
						*(ebpp[i]+1) - info.eip_fn_addr);
		}	
	}

/*	
uint32_t eip;
	eip = *(ebpp + 1);
        cprintf(" eip %08x ", eip);
*/
	// useless code
	/*
	asm volatile("movl 4(%%ebp), %0" : "=r" (eip));
	cprintf(" eip %08x ", eip);
	*/

/*
uint32_t args[5];
	cprintf(" args");
	for(int i = 0; i < 5; ++i){
		args[i] = *(ebpp + i + 2);	// for the first arg locates where 8 bytes above the ebp's position
		cprintf(" %08x", args[i]);
	}
	cprintf("\n");
*/

	// useless code
	/*
	//asm volatile("leal (%%ebp, %0, 4), %1" : "=r" (args) : "r" (i));
	//cprintf(" %08x", args);
	asm volatile("leal (%%ebp, %0, 4), %1 \n\t"
                                "movl (%1), %1 \n\t"
                                : "=r" (args)
                                : "r" (i)
                            );
	*/


	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
