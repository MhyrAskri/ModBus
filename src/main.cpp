
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <errno.h>

#include <modbus/modbus.h>

modbus_t *client_mb;
uint16_t tab_Hreg[5];
uint16_t tab_Ireg[5];
uint8_t tab_Creg[5];
uint8_t tab_Dreg[5];

modbus_t *server_mb;
modbus_mapping_t *server_mapping;

int rc;
int errno;
int soc = -1;

void Read_Modbus(int sig)
{
	signal(SIGALRM, SIG_IGN);
	printf("Modbus Read!!!\n");
	
	//read holding register
	rc=modbus_read_registers(client_mb, 102, 4, tab_Hreg);
	//write holding register
	//rc = modbus_write_register(ctx, UT_REGISTERS_ADDRESS, 0x1234);

	//test read results
	if (rc == -1) 
	{
		fprintf(stderr, "%s\n", modbus_strerror(errno));
	}
	printf("Holding Register:%u %u %u %u \n",tab_Hreg[0],tab_Hreg[1],tab_Hreg[2],tab_Hreg[3]);
	
	/*
	//read input register
	rc = modbus_read_input_registers(client_mb, 0, 4,tab_Ireg);
	//test read results
	if (rc == -1) 
	{
		fprintf(stderr, "%s\n", modbus_strerror(errno));
	}
	printf("Input Register:%u %u %u %u \n",tab_Ireg[0],tab_Ireg[1],tab_Ireg[2],tab_Ireg[3]);
	
	//read coil 
	rc = modbus_read_bits(client_mb, 0, 4, tab_Creg);
	//write coil 
	//rc = modbus_write_bit(ctx, UT_BITS_ADDRESS, ON);
	//test read results
	if (rc == -1) 
	{
		fprintf(stderr, "%s\n", modbus_strerror(errno));
	}
	printf("Coil Register:%u %u %u %u \n",tab_Creg[0],tab_Creg[1],tab_Creg[2],tab_Creg[3]);

	//read input status(DISCRITE)
	rc = modbus_read_input_bits(client_mb, 0, 4, tab_Dreg);
	//test read results
	if (rc == -1) 
	{
		fprintf(stderr, "%s\n", modbus_strerror(errno));
	}
	printf("Discrite Input:%u %u %u %u \n",tab_Dreg[0],tab_Dreg[1],tab_Dreg[2],tab_Dreg[3]);
	// update measurement values based on input string
	*/
	for(int i=0;i<4;i++)
	{
		//server_mapping->tab_registers[server_mapping->start_registers + i] =tab_Hreg[i];
		//server_mapping->tab_input_registers[server_mapping->start_input_registers+i] =tab_Ireg[i];
		//server_mapping->tab_bits[server_mapping->start_bits+i] =tab_Creg[i];
		//server_mapping->tab_input_bits[server_mapping->start_input_bits+i] =tab_Dreg[i];
		
		server_mapping->tab_registers[server_mapping->start_registers + i] =i;
		server_mapping->tab_input_registers[server_mapping->start_input_registers+i] =i;
		server_mapping->tab_bits[server_mapping->start_bits+i] =i;
		server_mapping->tab_input_bits[server_mapping->start_input_bits+i] =i;
	}

	signal(SIGALRM, Read_Modbus);
	alarm(3);
}


int main(void)
{
	

    //creat new modbus connection
   	//client_mb = modbus_new_tcp("10.3.13.17", 502);
	client_mb = modbus_new_rtu("/dev/ttyS2",9600,'N',8,1);

    if (client_mb == NULL) 
	{
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
	}

	modbus_set_slave(client_mb,1);

	rc=modbus_connect(client_mb);
	if (rc == -1) 
	{
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(client_mb);
		return -1;
	}
	if(client_mb != NULL && rc != -1)
	{
		fprintf(stderr,"Connect To Client Success!\n");
	}	

	

	//modbus server creating
	//tcp
	//server_mb = modbus_new_tcp_pi("::0", "503");
    //modbus_set_debug(server_mb, TRUE); 
	//rtu
	server_mb = modbus_new_rtu("/dev/ttyS2",9600,'N',8,1);

    //server_mapping = modbus_mapping_new_start_address(0, 0, 0, 0, 10000, 10, 0, 0);
	server_mapping = modbus_mapping_new(5,5,5,5);
    if (server_mapping == NULL) 
    {
        fprintf(stderr,"Failed to allocate the mapping: %s\n",modbus_strerror(errno));
        modbus_free(server_mb);
        return -1;
    }

	//active modbus client read register routing every few second
	signal(SIGALRM, Read_Modbus);
	alarm(3);
	//printf("while loop:\n");
    while(true) 
    {
			//printf("wait for query:\n");
			//tcp
			//u_int8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
			//rtu
			u_int8_t query[MODBUS_RTU_MAX_ADU_LENGTH];

			//printf("socket:%u\n",soc);

			//start server at startup
			if(soc == -1)
			{
				//tcp
				//soc = modbus_tcp_pi_listen(server_mb, 1);
				//modbus_tcp_pi_accept(server_mb, &soc);
				//rtu
				modbus_set_slave(server_mb, 2);
				soc = modbus_connect(server_mb);
			}
    		

			//printf("1socket:%u\n",soc);
			rc = modbus_receive(server_mb, query);
			//printf("1rc:%u\n",rc);

			if (rc > 0) 
			{
				// rc is the query size 
				modbus_reply(server_mb, query, rc, server_mapping);
			} 
			else if (rc == -1) 
			{
				
				// Connection closed by the client or error 
				close(soc);
				soc = -1;
				//break;
			}
    }

    
	fprintf(stderr,"Quit the loop: %s\n", modbus_strerror(errno));
	if (soc != -1) 
		{
			close(soc);
		}
    
    modbus_mapping_free(server_mapping);
	modbus_close(client_mb);
	modbus_free(client_mb);
    modbus_close(server_mb);
    modbus_free(server_mb);


    return 0;
}
