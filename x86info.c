/*
 *  (C) 2001-2009 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "x86info.h"
#include "mptable.h"

#include "Intel/Intel.h"

int show_bench=0;
int show_bios=0;
int show_bluesmoke=0;
int show_bugs=0;
int show_cacheinfo=0;
int show_connector=0;
int show_eblcr=0;
int show_msr=0;
int show_microcode=0;
int show_mtrr=0;
int show_pm=0;
int show_registers=0;
int show_urls=0;

static int show_mptable=0;
static int show_flags=0;
static int show_MHz=0;

static int num_sockets = 0;

int debug = 0;
int verbose = 0;
int silent = 0;
int used_UP = 0;
int user_is_root = 1;
static int need_root = 0;

unsigned int nrCPUs=1;
static unsigned int nrSMPCPUs;

static void usage (char *programname)
{
	printf("Usage: %s [<switches>]\n\
-a,   --all\n\
      --bench\n\
      --bios\n\
      --bugs\n\
-c,   --cache\n\
      --connector\n\
      --debug\n\
-f,   --flags\n\
-mhz, --mhz\n\
      --microcode\n\
-mp,  --mptable\n\
-m,   --msr\n\
      --mult\n\
      --mtrr\n\
      --pm\n\
-r,   --registers\n\
-s,   --show-bluesmoke\n\
-u,   --urls\n\
-v,   --verbose\n\
\n", programname);
	exit (0);
}

static void parse_command_line (int argc, char **argv)
{
	char **argp, *arg;

	for (argp = argv+1; argp <= argv + argc && (arg = *argp); argp++) {
		if ((!strcmp(arg, "-a") || !strcmp(arg, "--all"))) {
//			show_bench = 1;
			show_bios = 1;
			show_bluesmoke = 1;
			show_bugs = 1;
			show_cacheinfo = 1;
			show_connector = 1;
			show_eblcr =1;
			show_flags = 1;
			show_microcode = 1;
			show_mptable =1;
			show_msr = 1;
			show_MHz = 1;
			show_mtrr = 1;
			show_pm = 1;
			show_registers = 1;
			show_urls = 1;
			need_root = 1;
		}

		if (!strcmp(arg, "--bench"))
			show_bench = 1;

		if (!strcmp(arg, "--bios")) {
			need_root = 1;
			show_bios = 1;
		}

		if (!strcmp(arg, "--bugs"))
			show_bugs = 1;

		if ((!strcmp(arg, "-c") || !strcmp(arg, "--cache")))
			show_cacheinfo = 1;

		if (!strcmp(arg, "--connector"))
			show_connector = 1;

		if (!strcmp(arg, "--debug"))
			debug = 1;

		if ((!strcmp(arg, "-f") || !strcmp(arg, "--flags")))
			show_flags = 1;

		if ((!strcmp(arg, "-m") || !strcmp(arg, "--msr"))) {
			need_root = 1;
			show_msr = 1;
		}

		if (!strcmp(arg, "--microcode")) {
			need_root = 1;
			show_microcode = 1;
		}

		if ((!strcmp(arg, "-mhz") || !strcmp(arg, "--mhz")))
			show_MHz = 1;

		if ((!strcmp(arg, "-mp") || !strcmp(arg, "--mptable"))) {
			need_root = 1;
			show_mptable = 1;
		}

		if (!strcmp(arg, "--mtrr")) {
			need_root = 1;
			show_mtrr = 1;
		}

		if (!strcmp(arg, "--mult")) {
			need_root = 1;
			show_eblcr = 1;
		}

		if (!strcmp(arg, "--pm"))
			show_pm = 1;

		if ((!strcmp(arg, "-r") || !strcmp(arg, "--registers")))
			show_registers = 1;

		if ((!strcmp(arg, "-s") || !strcmp(arg, "--show-bluesmoke"))) {
			need_root = 1;
			show_bluesmoke = 1;
		}

		if ((!strcmp(arg, "-u") || !strcmp(arg, "--urls")))
			show_urls = 1;

		if ((!strcmp(arg, "-v") || !strcmp(arg, "--verbose")))
			verbose = 1;

		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);
	}
}


static void separator(void)
{
	int j;

	for (j=0; j<74; j++)
		printf("-");
	printf("\n");
}


int main (int argc, char **argv)
{
	unsigned int i;
	struct cpudata *cpu, *head=NULL, *tmp;
	int *sockets;

	parse_command_line(argc, argv);
	if (!silent) {
		printf("x86info v1.25.  Dave Jones 2001-2009\n");
		printf("Feedback to <davej@redhat.com>.\n\n");
	}

	if ((HaveCPUID()) == 0) {
		printf("No CPUID instruction available.\n");
		printf("No further information available for this CPU.\n");
		return 0;
	}

	if (getuid() != 0)
		user_is_root=0;

	if (need_root && !user_is_root)
		printf("Need to be root to use specified options.\n");

	nrCPUs = sysconf(_SC_NPROCESSORS_ONLN);

	if (!silent) {
		printf("Found %u CPU", nrCPUs);
		if (nrCPUs > 1)
			printf("s");

		/* Check mptable if present. This way we get number of CPUs
		   on SMP systems that have booted UP kernels. */
		if (user_is_root) {
			nrSMPCPUs = enumerate_cpus();
			if (nrSMPCPUs > nrCPUs)
				printf(", but found %ud CPUs in MPTable.", nrSMPCPUs);
		}
		printf("\n");
	}

	/*
	 * can't have less than 1 CPU, or more than
	 * 65535 (some arbitrary large number)
	 */
	if ((nrCPUs < 1) || (nrCPUs > 65535)) {
		printf("CPU count is bogus: defaulting to 1 CPU.\n");
		nrCPUs = 1;
	}

	if (show_mptable && user_is_root)
		(void)issmp(1);		// FIXME: issmp should become 'show_mptable'

	separator();

	/* Iterate over the linked list. */

	for (i=0; i<nrCPUs; i++) {
		cpu = malloc (sizeof (struct cpudata));
		if (!cpu) {
			printf("Out of memory\n");
			return -1;
		}

		memset(cpu, 0, sizeof(struct cpudata));

		if (!head) {
			head = cpu;
		} else {
			cpu->next = head;
			head = cpu;
		}

		cpu->number = i;

		if (!silent && nrCPUs != 1)
			printf("CPU #%u\n", i+1);

		bind_cpu(cpu);

		estimate_MHz(cpu);
		get_feature_flags(cpu);
		get_cpu_info_basics(cpu);	/* get vendor,family,model,stepping */
		identify(cpu);
		show_info(cpu);

		if (show_registers) {
			dumpregs(cpu->number, 0, cpu->cpuid_level);
			if (cpu->maxei >=0x80000000)
				dumpregs (cpu->number, 0x80000000, cpu->maxei);

			if (cpu->maxei2 >=0xC0000000)
				dumpregs (cpu->number, 0xC0000000, cpu->maxei2);
		}

		if (show_cacheinfo == 1) {
			switch (cpu->vendor) {
			case VENDOR_INTEL:
				decode_Intel_caches(cpu, 1);
				break;
			case VENDOR_AMD:
				decode_AMD_cacheinfo(cpu);
				break;
			default:
				break;
			}
		}

		if (show_flags == 1)
			show_feature_flags(cpu);

		if (show_connector)
			decode_connector(cpu->connector);

		if (show_urls) {
			if (cpu->datasheet_url != NULL)
				printf("Datasheet: %s\n", cpu->datasheet_url);
			if (cpu->errata_url != NULL)
				printf("Errata: %s\n", cpu->errata_url);
		}

		/* Info that requires root access (eg, reading MSRs etc) */
		if (user_is_root) {
			if (show_mtrr)
				dump_mtrrs(cpu);
		}

		/* Show MHz last. */
		if (show_MHz) {
			if (cpu->MHz < 1000)
				printf("%uMHz", cpu->MHz);
			else {
				int a = (cpu->MHz / 1000);
				int b = ((cpu->MHz % 1000)/100);
				int c = (a*1000)+(b*100);

				printf("%u.%u%uGHz", a, b, (cpu->MHz - c)/10);
			}
			printf(" processor (estimate).\n\n");
		}
		if (show_bench)
			show_benchmarks(cpu);

		if (nrCPUs > 1)
			separator();
	}

	if (debug == 1) {
		cpu = head;
		printf("cpu->phys_proc_id: ");
		for (i=0; i<nrCPUs; i++) {
			printf("%d, ", cpu->phys_proc_id);
			cpu = cpu->next;
		}
		printf("\n");

		cpu = head;
		printf("cpu->x86_max_cores: ");
		for (i=0; i<nrCPUs; i++) {
			printf("%d, ", cpu->x86_max_cores);
			cpu = cpu->next;
		}
		printf("\n");

		cpu = head;
		printf("cpu->cpu_core_id: ");
		for (i=0; i<nrCPUs; i++) {
			printf("%d, ", cpu->cpu_core_id);
			cpu = cpu->next;
		}
		printf("\n");
	}

	sockets = malloc(nrCPUs);
	if (sockets==NULL)
		exit(EXIT_FAILURE);
	bzero(sockets, nrCPUs);

	cpu = head;
	for (i=0; i<nrCPUs; i++) {
		sockets[cpu->phys_proc_id]++;
		cpu = cpu->next;
	}

	for (i=0; i<nrCPUs; i++) {
		if (sockets[i]==0)
			break;
		if (debug == 1)
			printf("Socket %d: %d threads\n", i, sockets[i]);
		num_sockets++;
	}

	cpu = head;
	printf("This is a %d socket ", num_sockets);
	if (cpu->flags_edx & X86_FEATURE_HT)
		printf("%d core processor with hyper-threading ", sockets[0]/2);
	else
		printf("%d core processor ", sockets[0]);
	printf("system.\n");

	/* Tear down the linked list. */
	cpu = head;
	for (i=0; i<nrCPUs; i++) {
		if (cpu->datasheet_url)
			free(cpu->datasheet_url);
		if (cpu->errata_url)
			free(cpu->errata_url);
		tmp = cpu->next;
		free(cpu);
		cpu = tmp;
	}

	if (nrCPUs > 1 && (used_UP == 1) && (!silent)) {
		printf("WARNING: Detected SMP, but unable to access cpuid driver.\n");
		printf("Used Uniprocessor CPU routines. Results inaccurate.\n");
	}

	return 0;
}
