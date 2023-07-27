// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
pte_t pte = uvpt[PGNUM(addr)];
	if(((err & FEC_WR) == 0) || ((pte & PTE_COW) == 0)){
		panic("faulting access is not a write or a copy-on-write page.");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
envid_t envid = sys_getenvid();
	if((r = sys_page_alloc(envid, (void*)PFTEMP, PTE_P | PTE_W | PTE_U)) != 0){
		panic("pgfault: %e", r);
	}
	memcpy((void*)PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
	if((r = sys_page_map(envid, PFTEMP, envid, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W)) != 0){
		panic("pgfault: %e", r);
	}
	if((r = sys_page_unmap(envid, PFTEMP)) != 0){
		panic("pgfault: %e", r);
	}

	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
void* addr = (void*)(pn * PGSIZE);
pte_t pte = uvpt[pn];
	if((pte & PTE_SHARE) == PTE_SHARE){
		if((r = sys_page_map(0, addr, envid, addr, (pte & PTE_SYSCALL))) != 0){
			panic("duppage: %e", r);
		}	
	}else if((pte & PTE_W) == PTE_W || (pte & PTE_COW) == PTE_COW){
		if((r = sys_page_map(0, addr, envid, addr, PTE_COW | PTE_P | PTE_U)) != 0){
			panic("duppage: %e", r);
		}
		if((r = sys_page_map(0, addr, 0, addr, PTE_COW | PTE_P | PTE_U))){
			panic("duppage: %e", r);
		}	
	}else{
		if((r = sys_page_map(0, addr, envid, addr, PTE_P | PTE_U)) != 0){
			panic("duppage: %e", r);
		}	
	}
	
	// if((pte & PTE_W) == PTE_W || (pte & PTE_COW) == PTE_COW){
	// 	if((r = sys_page_map(0, addr, envid, addr, PTE_COW | PTE_P | PTE_U)) != 0){
	// 		panic("duppage: %e", r);
	// 	}
	// 	if((r = sys_page_map(0, addr, 0, addr, PTE_COW | PTE_P | PTE_U))){
	// 		panic("duppage: %e", r);
	// 	}
	// }else{
	// 	if((r = sys_page_map(0, addr, envid, addr, PTE_P | PTE_W | PTE_U)) != 0){
	// 		panic("duppage: %e", r);
	// 	}
	// }
	// panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
int r;
envid_t child_envid;
	set_pgfault_handler(pgfault);

	child_envid = sys_exofork();
	if(child_envid == 0){
		// we are child
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
uint32_t start = PGNUM(UTEXT);
uint32_t end = PGNUM(USTACKTOP);
	for(uint32_t i = start; i < end; ++i){
		if((uvpd[i >> 10] & PTE_P) && (uvpt[i] & PTE_P)){
			if((r = duppage(child_envid, i)) != 0){
				return r;
			}
		}
	}
	if((r = sys_page_alloc(child_envid, (void*)(UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W)) != 0){
		panic("fork: %e", r);
	}
extern void _pgfault_upcall(void);
	if((r = sys_env_set_pgfault_upcall(child_envid, _pgfault_upcall)) != 0){
		panic("fork, sys_env_set_pgfault_upcall: %e", r);
	}
	if((r = sys_env_set_status(child_envid, ENV_RUNNABLE)) != 0){
		return r;
	}
	return child_envid;
	// panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
