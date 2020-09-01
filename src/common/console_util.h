struct sIconLUT {
	int texture;
	unsigned int color;
	int slot;
};

sIconLUT iconLUT[300];

void SetIconLUT() {
	for(int i=0; i<300; i++) iconLUT[i].slot = -1;
	//comms
	iconLUT[127].texture	= 0;
	iconLUT[127].color		= 0x080;
	iconLUT[127].slot		= 3;

	iconLUT[164].texture	= 0;
	iconLUT[164].color		= 0x0F0;
	iconLUT[164].slot		= 3;
	//navs
	iconLUT[128].texture	= 0;
	iconLUT[128].color		= 0x080;
	iconLUT[128].slot		= 2;

	iconLUT[165].texture	= 0;
	iconLUT[165].color		= 0x0F0;
	iconLUT[165].slot		= 2;
	//info
	iconLUT[129].texture	= 0;
	iconLUT[129].color		= 0x080;
	iconLUT[129].slot		= 1;

	iconLUT[166].texture	= 0;
	iconLUT[166].color		= 0x0F0;
	iconLUT[166].slot		= 1;
	//views
	iconLUT[130].texture	= 0;
	iconLUT[130].color		= 0x080;
	iconLUT[130].slot		= 0;

	iconLUT[167].texture	= 0;
	iconLUT[167].color		= 0x0F0;
	iconLUT[167].slot		= 0;
	//battle-nav
	iconLUT[163].texture	= 0;
	iconLUT[163].color		= 0x0F0;
	iconLUT[163].slot		= 4;
	
	iconLUT[178].texture	= 0;
	iconLUT[178].color		= 0xF00;
	iconLUT[178].slot		= 4;
	//front view
	iconLUT[162].texture	= 0;
	iconLUT[162].color		= 0x080;
	iconLUT[162].slot		= 5;

	iconLUT[177].texture	= 0;
	iconLUT[177].color		= 0x0F0;
	iconLUT[177].slot		= 5;
	//rear view
	iconLUT[161].texture	= 0;
	iconLUT[161].color		= 0x080;
	iconLUT[161].slot		= 6;

	iconLUT[176].texture	= 0;
	iconLUT[176].color		= 0x0F0;
	iconLUT[176].slot		= 6;
	//turret view
	iconLUT[160].texture	= 0;
	iconLUT[160].color		= 0x080;
	iconLUT[160].slot		= 7;

	iconLUT[175].texture	= 0;
	iconLUT[175].color		= 0x0F0;
	iconLUT[175].slot		= 7;
	//external view
	iconLUT[159].texture	= 0;
	iconLUT[159].color		= 0x080;
	iconLUT[159].slot		= 8;

	iconLUT[174].texture	= 0;
	iconLUT[174].color		= 0x0F0;
	iconLUT[174].slot		= 8;
	//missile view
	iconLUT[158].texture	= 0;
	iconLUT[158].color		= 0x080;
	iconLUT[158].slot		= 9;

	iconLUT[173].texture	= 0;
	iconLUT[173].color		= 0x0F0;
	iconLUT[173].slot		= 9;
	//combat comp
	iconLUT[157].texture	= 0;
	iconLUT[157].color		= 0x080;
	iconLUT[157].slot		= 10;

	iconLUT[172].texture	= 0;
	iconLUT[172].color		= 0x0F0;
	iconLUT[172].slot		= 10;
	//ejecttion pod
	iconLUT[156].texture	= 0;
	iconLUT[156].color		= 0x080;
	iconLUT[156].slot		= 11;

	iconLUT[171].texture	= 0;
	iconLUT[171].color		= 0x0F0;
	iconLUT[171].slot		= 11;
	//ports
	iconLUT[155].texture	= 0;
	iconLUT[155].color		= 0x080;
	iconLUT[155].slot		= 12;

	iconLUT[170].texture	= 0;
	iconLUT[170].color		= 0x0F0;
	iconLUT[170].slot		= 12;
	//labels
	iconLUT[154].texture	= 0;
	iconLUT[154].color		= 0x080;
	iconLUT[154].slot		= 13;

	iconLUT[169].texture	= 0;
	iconLUT[169].color		= 0x0F0;
	iconLUT[169].slot		= 13;
	//red warning
	iconLUT[212].texture	= 0;
	iconLUT[212].color		= 0x400;
	iconLUT[212].slot		= 14;

	iconLUT[213].texture	= 0;
	iconLUT[213].color		= 0xf00;
	iconLUT[213].slot		= 14;
	//green warning
	iconLUT[214].texture	= 0;
	iconLUT[214].color		= 0x040;
	iconLUT[214].slot		= 15;

	iconLUT[215].texture	= 0;
	iconLUT[215].color		= 0x0F0;
	iconLUT[215].slot		= 15;
	//blue warning
	iconLUT[216].texture	= 0;
	iconLUT[216].color		= 0x004;
	iconLUT[216].slot		= 16;

	iconLUT[217].texture	= 0;
	iconLUT[217].color		= 0x00F;
	iconLUT[217].slot		= 16;
	//orange gage
	iconLUT[273].texture	= 0;
	iconLUT[273].color		= 0xC80;
	iconLUT[273].slot		= 17;
	//red gage
	iconLUT[274].texture	= 0;
	iconLUT[274].color		= 0xF00;
	iconLUT[274].slot		= 18;
	//green gage
	iconLUT[275].texture	= 0;
	iconLUT[275].color		= 0x0F0;
	iconLUT[275].slot		= 19;
	//red stop
	iconLUT[253].texture	= 0;
	iconLUT[253].color		= 0xF00;
	iconLUT[253].slot		= 25;
	//pause
	iconLUT[243].texture	= 0;
	iconLUT[243].color		= 0x080;
	iconLUT[243].slot		= 25;

	iconLUT[252].texture	= 0;
	iconLUT[252].color		= 0x0F0;
	iconLUT[252].slot		= 25;
	//play 1x
	iconLUT[242].texture	= 0;
	iconLUT[242].color		= 0x080;
	iconLUT[242].slot		= 24;

	iconLUT[251].texture	= 0;
	iconLUT[251].color		= 0x0F0;
	iconLUT[251].slot		= 24;
	//play 2x
	iconLUT[241].texture	= 0;
	iconLUT[241].color		= 0x080;
	iconLUT[241].slot		= 23;

	iconLUT[250].texture	= 0;
	iconLUT[250].color		= 0x0F0;
	iconLUT[250].slot		= 23;
	//play 3x
	iconLUT[240].texture	= 0;
	iconLUT[240].color		= 0x080;
	iconLUT[240].slot		= 22;

	iconLUT[249].texture	= 0;
	iconLUT[249].color		= 0x0F0;
	iconLUT[249].slot		= 22;
	//play 4x
	iconLUT[239].texture	= 0;
	iconLUT[239].color		= 0x080;
	iconLUT[239].slot		= 21;

	iconLUT[248].texture	= 0;
	iconLUT[248].color		= 0x0F0;
	iconLUT[248].slot		= 21;
	//play 5x
	iconLUT[238].texture	= 0;
	iconLUT[238].color		= 0x080;
	iconLUT[238].slot		= 20;

	iconLUT[247].texture	= 0;
	iconLUT[247].color		= 0x0F0;
	iconLUT[247].slot		= 20;
	//eng
	iconLUT[264].texture	= 0;
	iconLUT[264].color		= 0x080;
	iconLUT[264].slot		= 26;

	iconLUT[263].texture	= 0;
	iconLUT[263].color		= 0x0F0;
	iconLUT[263].slot		= 26;
	//gears
	iconLUT[265].texture	= 0;
	iconLUT[265].color		= 0x080;
	iconLUT[265].slot		= 27;

	iconLUT[266].texture	= 0;
	iconLUT[266].color		= 0x0F0;
	iconLUT[266].slot		= 27;
	//warning shield
	iconLUT[262].texture	= 0;
	iconLUT[262].color		= 0xF00;
	iconLUT[262].slot		= 27;
	//hyperdrive
	iconLUT[267].texture	= 0;
	iconLUT[267].color		= 0x0F0;
	iconLUT[267].slot		= 28;
	//target
	iconLUT[287].texture	= 0;
	iconLUT[287].color		= 0x080;
	iconLUT[287].slot		= 29;

	iconLUT[280].texture	= 0;
	iconLUT[280].color		= 0x0F0;
	iconLUT[280].slot		= 29;	
	//combat background
	iconLUT[268].texture	= 1;
	iconLUT[268].color		= 0xF00;
	iconLUT[268].slot		= 30;
	//combat target
	iconLUT[288].texture	= 2;
	iconLUT[288].color		= 0xF00;
	iconLUT[288].slot		= 31;
	//ecm
	iconLUT[284].texture	= 2;
	iconLUT[284].color		= 0xF00;
	iconLUT[284].slot		= 32;
	//naval ecm
	iconLUT[272].texture	= 2;
	iconLUT[272].color		= 0xF00;
	iconLUT[272].slot		= 32;
	//chaff disp.
	iconLUT[271].texture	= 2;
	iconLUT[271].color		= 0xF00;
	iconLUT[271].slot		= 33;
	//hyp.cloud analyser
	iconLUT[283].texture	= 2;
	iconLUT[283].color		= 0xF00;
	iconLUT[283].slot		= 34;
	//energy bomb
	iconLUT[269].texture	= 2;
	iconLUT[269].color		= 0xF00;
	iconLUT[269].slot		= 36;

}