/* x86info by Dave Jones
 * Based on 'cpuid' by Phil Karn, KA9Q
 * May be used under the terms of the GNU Public License (GPL)
 */

#include <linux/types.h>

typedef __u32 u32;

#define VENDOR_AMD	1
#define VENDOR_CENTAUR 2
#define VENDOR_CYRIX 3
#define VENDOR_INTEL 4
#define VENDOR_RISE 5

struct cpudata {
	int vendor;
	int family;
	int model;
	int stepping;
	int type;
	int brand;
	int cachesize_L1;
	int cachesize_L2;
	char name[80];
};

extern int used_UP;

void cpuid (int, int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);
void cpuid_UP (int, unsigned long *, unsigned long *, unsigned long *, unsigned long *);
void decode_intel_tlb (int);
void decode_cyrix_tlb (int);
void Identify_Intel (int, unsigned int, struct cpudata *cpu);
void Identify_AMD (int, unsigned int, struct cpudata *cpu);
void Identify_Cyrix (int, unsigned int, unsigned int, struct cpudata *cpu);
void Identify_IDT (int, unsigned int, unsigned int, struct cpudata *cpu);
void Identify_Rise (int, unsigned int, unsigned int, struct cpudata *cpu);

void display_AMD_info(int cpunum, unsigned int maxei, struct cpudata *cpu);
void display_Cyrix_info(int cpunum, unsigned int maxi, unsigned int maxei, struct cpudata *cpu);
void display_IDT_info(int cpunum, unsigned int maxei, struct cpudata *cpu);
void display_Intel_info(int cpunum, unsigned int maxi, struct cpudata *cpu);
void display_Rise_info(int cpunum, unsigned int maxi, unsigned int maxei, struct cpudata *cpu);

void decode_feature_flags (struct cpudata *cpu, int flags);
void identify (int cpunum);
void dumpregs (int cpunum);

int rdmsr(int cpu, unsigned int index, unsigned long long *val);
void long2binstr(long value);
void dumpmsr (int cpunum, unsigned int msr);
void dumpmsr_bin (int cpunum, unsigned int msr);
void estimate_MHz(int cpunum);
void decode_bluesmoke(int cpunum);
int HaveCPUID(void);
void interpret_eblcr(u32 lo);

