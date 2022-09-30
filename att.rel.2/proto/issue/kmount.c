main(argc,argv)
char **argv;
{
	if(argv[0][1] == 'm')
		mount(argv[1],argv[2],0);
	else
		umount ( argv[1] );
}
