//
// Created by Meluleki Dube on 14/02/2021.
//

#include "CUnit/Basic.h"
#include "shell.h"

#define TRUE 1
#define FALSE 0
Array_(string, test_commands) = {
"cd temp",
"find ./ -name \\*.html -printf '%CD\\t%p\n\' | grep \"03/10/08\" | awk \'{print $2}\' | xargs -t -i mv {} temp/",
"agetty -L 9600 ttyS1 vt100",
"alias home='cd /home/tecmint/public_html'",
"apropos adduser",
"sudo echo \"shutdown -h now\" | at -m 23:55",
"awk \'//{print}\'/etc/hosts",
"echo “This is TecMint - Linux How Tos”",
"grep ‘tecmint’ domain-list.txt",
"cat file1 file2 | gzip > foo.gz",
"ps -eo pid,ppid,cmd,%mem,%cpu --sort=-%mem | head",
"history",
"ln -s /usr/bin/lscpu cpuinfo",
"mkdir tecmint-files",
"tar cf - . | zip | dd of=/dev/nrst0 obs=16k",
"zip inarchive.zip foo.c bar.c --out outarchive.zip",
"tar cf - .| zip backup -"
};

Array_(string, expected_command_names) = {
        "",
};

Array_(int, expected_argc) = {

};

Array_(int, exptected_parse_output) = {

};

#ifdef TEST
int main() {
      // Initialize the CUnit test registry
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();
   // Sets the basic run mode, CU_BRM_VERBOSE will show maximum output of run details
   // Other choices are: CU_BRM_SILENT and CU_BRM_NORMAL
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_pSuite basic_operations_suit = NULL, other_operatios_suit= NULL;
    // Add a suite to the registry
    // Always check if add was successful
   // Run the tests and show the run summary
   CU_basic_run_tests();
   return CU_get_error();
}
#endif