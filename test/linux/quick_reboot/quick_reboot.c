/** \file
 * \brief Example code for Simple Open EtherCAT master
 *
 * Usage: quick_reboot ifname1 slave
 * ifname is NIC interface, f.e. eth0
 * slave = slave number in EtherCAT order 1..n
 *
 * This is a slave quick_reboot test.
 *
 * (c)Arthur Ketels 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ethercat.h"

uint32 data;
uint16 argslave;

void quick_reboot(char *ifname, uint16 slave)
{
    printf("Starting quick_reboot\n");

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded\n", ifname);
        /* find and auto-config slaves */

        if (ec_config_init(FALSE) > 0)
        {
            printf("%d slaves found and configured\n", ec_slavecount);

            /* wait for slave to reach PRE_OP state */
            if (ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE * 4) == EC_STATE_PRE_OP)
                printf("Slave %d reach PRE_OP state\n", slave);
            else
                printf("Slave %d does not reach PRE_OP state\n", slave);

            printf("Request INIT state for slave %d\n", slave);
            ec_slave[slave].state = EC_STATE_INIT;
            ec_writestate(slave);

            /* wait for slave to reach INIT state */
            if (ec_statecheck(0, EC_STATE_INIT, EC_TIMEOUTSTATE * 4) == EC_STATE_INIT)
                printf("Slave %d reach INIT state\n", slave);
            else
                printf("Slave %d does not reach INIT state\n", slave);

            /* read BOOT mailbox data, master -> slave */
            data = ec_readeeprom(slave, ECT_SII_BOOTRXMBX, EC_TIMEOUTEEP);
            ec_slave[slave].SM[0].StartAddr = (uint16)LO_WORD(data);
            ec_slave[slave].SM[0].SMlength = (uint16)HI_WORD(data);
            /* store boot write mailbox address */
            ec_slave[slave].mbx_wo = (uint16)LO_WORD(data);
            /* store boot write mailbox size */
            ec_slave[slave].mbx_l = (uint16)HI_WORD(data);

            /* read BOOT mailbox data, slave -> master */
            data = ec_readeeprom(slave, ECT_SII_BOOTTXMBX, EC_TIMEOUTEEP);
            ec_slave[slave].SM[1].StartAddr = (uint16)LO_WORD(data);
            ec_slave[slave].SM[1].SMlength = (uint16)HI_WORD(data);
            /* store boot read mailbox address */
            ec_slave[slave].mbx_ro = (uint16)LO_WORD(data);
            /* store boot read mailbox size */
            ec_slave[slave].mbx_rl = (uint16)HI_WORD(data);

            printf(" SM0 A:%4.4x L:%4d F:%8.8x\n", ec_slave[slave].SM[0].StartAddr, ec_slave[slave].SM[0].SMlength,
                   (int)ec_slave[slave].SM[0].SMflags);
            printf(" SM1 A:%4.4x L:%4d F:%8.8x\n", ec_slave[slave].SM[1].StartAddr, ec_slave[slave].SM[1].SMlength,
                   (int)ec_slave[slave].SM[1].SMflags);
            /* program SM0 mailbox in for slave */
            ec_FPWR(ec_slave[slave].configadr, ECT_REG_SM0, sizeof(ec_smt), &ec_slave[slave].SM[0], EC_TIMEOUTRET);
            /* program SM1 mailbox out for slave */
            ec_FPWR(ec_slave[slave].configadr, ECT_REG_SM1, sizeof(ec_smt), &ec_slave[slave].SM[1], EC_TIMEOUTRET);

            printf("Request BOOT state for slave %d\n", slave);
            ec_slave[slave].state = EC_STATE_BOOT;
            ec_writestate(slave);

            /* wait for slave to reach BOOT state */
            if (ec_statecheck(slave, EC_STATE_BOOT, EC_TIMEOUTSTATE * 10) == EC_STATE_BOOT)
                printf("Slave %d reach BOOT state\n", slave);
            else
                printf("Slave %d does not reach BOOT state\n", slave);

            printf("Request INIT state for slave %d, transition BOOT_TO_INIT triggers slave to restart\n", slave);
            ec_slave[slave].state = EC_STATE_INIT;
            ec_writestate(slave);
        }
        else
        {
            printf("No slaves found!\n");
        }
        printf("End quick_reboot, close socket\n");
        /* stop SOEM, close socket */
        ec_close();
    }
    else
    {
        printf("No socket connection on %s\nExcecute as root\n", ifname);
    }
}

int main(int argc, char *argv[])
{
    printf("SOEM (Simple Open EtherCAT Master)\nquick_reboot\n");

    if (argc > 2)
    {
        argslave = atoi(argv[2]);
        quick_reboot(argv[1], argslave);
    }
    else
    {
        printf("Usage: firm_update ifname1 slave fname\n");
        printf("ifname = eth0 for example\n");
        printf("slave = slave number in EtherCAT order 1..n\n");
    }

    printf("End program\n");
    return (0);
}
