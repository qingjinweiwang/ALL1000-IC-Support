
module	H3V3SO8UG4A3(
					FPGAGCLK0,FPGAGCLK1,
					FPGAAddr,
					FPGAData,
					FPGAWR,FPGARD,
				
			//////////// All Socket Same Share signal ////////////////
					

			/////////////// Socket A signal //////////////////////////
					SDIPortA,SDOPortA,SCKPortA,
					CSPortA,RstPortA,WPPortA,HoldPortA,
					GndPortA,VccPortA,

			/////////////// Socket B signal //////////////////////////
					SDIPortB,SDOPortB,SCKPortB,
					CSPortB,RstPortB,WPPortB,HoldPortB,
					GndPortB,VccPortB,

			/////////////// Socket C signal //////////////////////////
					SDIPortC,SDOPortC,SCKPortC,
					CSPortC,RstPortC,WPPortC,HoldPortC,
					GndPortC,VccPortC,

			/////////////// Socket D signal //////////////////////////
					SDIPortD,SDOPortD,SCKPortD,
					CSPortD,RstPortD,WPPortD,HoldPortD,
					GndPortD,VccPortD,

					) ;


/********************************************************************/
/*           Each Socket VCC & GND definition setting               */
/********************************************************************/





/********************************************************************/
/*             Driver input/output definition setting               */
/********************************************************************/
input wire FPGAGCLK0,FPGAGCLK1;
reg SYSCLKReg ;
input wire [15:0] FPGAAddr;
inout wire [15:0] FPGAData;
input wire FPGAWR, FPGARD;


/********************************************************************/
/*            All Socket Same signal                                */
/********************************************************************/



/********************************************************************/
/*            Socket A signal                                       */
/********************************************************************/
input wire SDOPortA ;
output wire SDIPortA,SCKPortA,CSPortA,HoldPortA ;
inout wire RstPortA,WPPortA ;
inout wire GndPortA ;
inout wire VccPortA ;

/********************************************************************/
/*            Socket B signal                                       */
/********************************************************************/
input wire SDOPortB ;
output wire SDIPortB,SCKPortB,CSPortB,HoldPortB ;
inout wire RstPortB,WPPortB ;
inout wire GndPortB ;
inout wire VccPortB ;

/********************************************************************/
/*            Socket C signal                                       */
/********************************************************************/
input wire SDOPortC ;
output wire SDIPortC,SCKPortC,CSPortC,HoldPortC ;
inout wire RstPortC,WPPortC ;
inout wire GndPortC ;
inout wire VccPortC ;

/********************************************************************/
/*            Socket D signal                                       */
/********************************************************************/
input wire SDOPortD ;
output wire SDIPortD,SCKPortD,CSPortD,HoldPortD ;
inout wire RstPortD,WPPortD ;
inout wire GndPortD ;
inout wire VccPortD ;


/********************************************************************/
/*            system register                                       */
/********************************************************************/
reg [5:0] StateReg ;
reg [7:0] FpgaDataReg ;
reg [7:0] DataRegA ;
reg [7:0] DataRegB ;
reg [7:0] DataRegC ;
reg [7:0] DataRegD ;
reg [31:0] AddrReg, AddrBackReg ;
reg RstReg ;


/********************************************************************/
/*		user define register here                                   */
/********************************************************************/
reg SDIReg ;
reg SCKReg ;
reg CSReg ;
reg RdWrFlag ;
reg EnSCKOut ;
reg[7:0] ClockStep ;


/********************************************************************/
/*		user define parameter here                                  */
/********************************************************************/
parameter	StartCnt		= 1'b0 ;
parameter	EndCnt			= 1'b1 ;
parameter	ReadData		= 1'b1 ;
parameter	WriteData		= 1'b0 ;



/********************************************************************/
/******* below for Gang system control setting **********************/
/********************************************************************/
reg[15:0] GangChkSckStatus, GangChkSckStatusNext ; /* 0 = socket check pass, 1 = socket check fail */
                            /* Gang Data for check status fee-back to CPU */
reg[15:0] GangErrSckMask ; /* 1 = check error or disable socket, 0 = check pass or enable socket */
parameter	ChkAllSckErrFlag	= 8'hFF ;
parameter	ChkSckAErrFlag		= 8'h01 ;
parameter	ChkSckBErrFlag		= 8'h02 ;
parameter	ChkSckCErrFlag		= 8'h04 ;
parameter	ChkSckDErrFlag		= 8'h08 ;
parameter	ChkSckEErrFlag		= 8'h10 ;
parameter	ChkSckFErrFlag		= 8'h20 ;
parameter	ChkSckGErrFlag		= 8'h40 ;
parameter	ChkSckHErrFlag		= 8'h80 ;

reg[7:0] EnSelSocketNum ; /* select socket number to get data */
parameter	EnAllSckCtrlFlag		= 8'hFF ;
parameter	EnSckACtrlFlag			= 8'h01 ;
parameter	EnSckBCtrlFlag			= 8'h02 ;
parameter	EnSckCCtrlFlag			= 8'h04 ;
parameter	EnSckDCtrlFlag			= 8'h08 ;
parameter	EnSckECtrlFlag			= 8'h10 ;
parameter	EnSckFCtrlFlag			= 8'h20 ;
parameter	EnSckGCtrlFlag			= 8'h40 ;
parameter	EnSckHCtrlFlag			= 8'h80 ;

reg[15:0] GDevDataForChk ; /* temp save in FPGA for checking */
reg[15:0] GDevMaskData ; /* temp save in FPGA for checking mask */




/********************************************************************/
/*                     Macro definition setting                     */
/* Port 0x0000 ~ 0x0FFF for Device Data setting decode              */
/********************************************************************/
/********* Nor(ADM) Flash *********************/
parameter	DevDataLoInc0		= 16'h0000 ;
parameter	DevDataLoInc1		= 16'h0100 ;
parameter	DevDataLoDec1		= 16'h0200 ;
parameter	DevCmdxAddr55		= 16'h0500 ;
parameter	DevAddrDataCmd		= 16'h0600 ; // for ADM nor flash */
parameter	DevCmdxAddrAA		= 16'h0A00 ;
parameter	GDevChkDataLoInc0	= 16'h0E00 ;
parameter	GDevChkDataLoInc1	= 16'h0F00 ;


/***************** Nand Flash **********************/
parameter	DevDataCmd			= 16'h0000 ;
parameter	DevCtrlCmd			= 16'h0500 ;
parameter	DevAddrCmd			= 16'h0600 ;


/***************** SPI Flash **********************/
parameter	StartReadData		= 16'h0000 ;
parameter	OneDataCmd			= 16'h0100 ; /* for SPI, Read Device Data from Fpga */


/********************************************************************/
/*                     Macro definition setting                     */
/* Port 0x8000 ~ 0xFFFF for FPGA & Device Parameter decode          */
/********************************************************************/
parameter	SetFpgaPara			= 16'h8000 ;
parameter	SetRSTPinCmd		= 16'h8000 ; /* 0x8000 for pin control */
parameter	SetWPPinCmd		= 16'h8100 ;
parameter	SetADVPinCmd		= 16'h8200 ;
parameter	SetCSPinCmd			= 16'h8300 ;
parameter	SetCLEPinCmd		= 16'h8400 ;
parameter	SetALEPinCmd		= 16'h8500 ;

parameter	SetAddrLoWord		= 16'h9000 ; /* for device parameter setting */
parameter	SetAddrHiWord		= 16'h9100 ;
parameter	SetDevPageLth		= 16'h9200 ;
parameter	DevAddxAddr			= 16'h9300 ;

parameter	GSetAllErrSckMask	= 16'hA000 ; /* set fpga all error or NoUse socket mask flag */
parameter	GSetAllMaskDataLo	= 16'hA100 ; /* mask data check bits */
parameter	GEnSelSckNum		= 16'hA200 ;
parameter	GChkAllSckStatus	= 16'hA300 ; /* fpga checking data and fee-back result, 0=pass, 1=fail */
parameter	GSetPageChkCnt		= 16'hA400 ;
parameter	GClrPageChkStatus	= 16'hA500 ; /* clear page check status */
parameter	GGetSckxData		= 16'hA600 ;


//////////////////////////////////////////////////////////
parameter	FPGAEnAllPin			= 16'hE000 ;
parameter	FPGAComChk5A5A		= 16'hE000 ; /* check to confirm FPGA communication prot ok */
parameter	FPGAComChkA5A5		= 16'hF000 ; /* check to confirm FPGA communication prot ok */
parameter	FPGADisAllPin			= 16'hF000 ;



/**********************************************************************/
/******** Port 0x7000~0x7FFF for DIO Mode Pin Setting Define  *********/
/***** DIO mode use Byte width FPGA setting !!!!              *********/
/**********************************************************************/
parameter	SetClkPinFreq		= 16'h7000 ;
parameter	VccPinMinNum		= 16'h7001 ; /* 0x01 ~ 0xC0 for Pin1~Pin192 control */
parameter	VccPinMaxNum		= 16'h70C0 ;
parameter	VppPinMinNum		= 16'h7101 ; /* 0x01 ~ 0x20 for Pin1~Pin32 control */
parameter	VppPinMaxNum		= 16'h7120 ;
parameter	VioPinMinNum		= 16'h7201 ; /* 0x01 ~ 0x20 for Pin1~Pin32 control */
parameter	VioPinMaxNum		= 16'h7220 ;
parameter	DirPinMinNum		= 16'h7301 ; /* 0x01 ~ 0xC0 for Pin1~Pin192 control */
parameter	DirPinMaxNum		= 16'h73C0 ;
parameter	ZPinMinNum			= 16'h7401 ; /* 0x01 ~ 0xC0 for Pin1~Pin192 control */
parameter	ZPinMaxNum			= 16'h74C0 ;
parameter	ClkPinMinNum		= 16'h7501 ; /* 0x01 ~ 0xC0 for Pin1~Pin192 control */
parameter	ClkPinMaxNum		= 16'h75C0 ;
parameter	GndPinMinNum		= 16'h7601 ; /* 0x01 ~ 0xC0 for Pin1~Pin192 control */
parameter	GndPinMaxNum		= 16'h76C0 ;

parameter	FPGACFGPort			= 16'h7FFF ; /* for FPGA CFG setting, CANNOT for other use!!! */




/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
parameter	EnFreqOut		= 1'b1 ;
parameter	DisFreqOut		= 1'b0 ;



/*******************************************/
/******** FSMD Parameter *******************/
/******************************************/
parameter	IdleState		= 6'b000000 ; // FSM-Start

parameter	ReadDataLo		= 6'b000001 ; // group 0
parameter	ReadDataHi		= 6'b000010 ;
parameter	ReadDataEnd0	= 6'b000011 ;
parameter	ReadDataEnd1	= 6'b000100 ;

parameter	WriteDataLo		= 6'b001001 ; // group 1
parameter	WriteDataHi		= 6'b001010 ;
parameter	WriteDataEnd0	= 6'b001011 ;
parameter	WriteDataEnd1	= 6'b001100 ;

parameter	ReadGangData	= 6'b010001 ; // group 2
parameter	ReadGangEnd0	= 6'b010010 ;
parameter	ReadGangEnd1	= 6'b010011 ;
parameter	WriteGangCmd	= 6'b010100 ;
parameter	WriteGangEnd0	= 6'b010101 ;
parameter	WriteGangEnd1	= 6'b010110 ;

parameter	ReadSetting		= 6'b011001 ; // group 3
parameter	ReadSetEnd		= 6'b011010 ;
parameter	WriteSetting	= 6'b011011 ;
parameter	WriteSetEnd		= 6'b011100 ;
parameter	RDWRSetIdle		= 6'b011101 ;

parameter	ReadVccStatus	= 6'b100001 ; // group 4
parameter	ReadVccEnd		= 6'b100010 ;
parameter	ReadGndStatus	= 6'b100011 ;
parameter	ReadGndEnd		= 6'b100100 ;

parameter	BusyState		= 6'b111111 ; // FSM-End




/********************************************************************/
/*       Main driver for FPGA 144 pin drive control initial          */
/********************************************************************/
//////////// first initial system when power on /////////////////////
initial
  begin
	GangChkSckStatus <= 16'b0 ;
	GangChkSckStatusNext <= 16'b0 ;
	GangErrSckMask <= 16'b1 ; /* default set all pass mask status */
	GDevDataForChk <= 16'b0 ;
	GDevMaskData <= 16'b1 ;
	EnSelSocketNum <= EnAllSckCtrlFlag ; /* set default to enable all socket data */

	StateReg <= IdleState ; /* MUST define together */
	AddrReg <= 32'b0 ;
	AddrBackReg <= 32'b0 ;
	RstReg <= 1'b1 ;

	SCKReg = 1'b0 ;
	CSReg = 1 ;
	EnSCKOut = EndCnt ;
	ClockStep = 0 ; /* re-initial wave-step to first status */
  end


/********************************************************************/
/*       Main driver for FPGA 144 pin drive control setting         */
/********************************************************************/
// FSMD = Fpga State Machine Driving & data registers control
always @(posedge FPGAGCLK0) /* System Clock = 66MHz */
  begin
	////////////////////////////////////////////////////////////////////
	if( EnSCKOut == StartCnt )
	  begin
		if( RdWrFlag == ReadData ) /* read data from device */
		  begin
			if( ClockStep == 1 )
			  begin
				DataRegA[7] <= SDOPortA ;
				DataRegB[7] <= SDOPortB ;
				DataRegC[7] <= SDOPortC ;
				DataRegD[7] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 4 )
			  begin
				DataRegA[6] <= SDOPortA ;
				DataRegB[6] <= SDOPortB ;
				DataRegC[6] <= SDOPortC ;
				DataRegD[6] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 7 )
			  begin
				DataRegA[5] <= SDOPortA ;
				DataRegB[5] <= SDOPortB ;
				DataRegC[5] <= SDOPortC ;
				DataRegD[5] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 10 )
			  begin
				DataRegA[4] <= SDOPortA ;
				DataRegB[4] <= SDOPortB ;
				DataRegC[4] <= SDOPortC ;
				DataRegD[4] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 13 )
			  begin
				DataRegA[3] <= SDOPortA ;
				DataRegB[3] <= SDOPortB ;
				DataRegC[3] <= SDOPortC ;
				DataRegD[3] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 16 )
			  begin
				DataRegA[2] <= SDOPortA ;
				DataRegB[2] <= SDOPortB ;
				DataRegC[2] <= SDOPortC ;
				DataRegD[2] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 19 )
			  begin
				DataRegA[1] <= SDOPortA ;
				DataRegB[1] <= SDOPortB ;
				DataRegC[1] <= SDOPortC ;
				DataRegD[1] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 22 )
			  begin
				DataRegA[0] <= SDOPortA ;
				DataRegB[0] <= SDOPortB ;
				DataRegC[0] <= SDOPortC ;
				DataRegD[0] <= SDOPortD ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep == 23 )
				SCKReg = 1'b0 ;
			else if( ClockStep < 24 )
				SCKReg = 1'b1 ;

	
			if( ClockStep < 24 )
				ClockStep = ClockStep + 1 ;
			else if( ClockStep == 24 )
			  begin
				/********* Check 4 Socket data after read finish ******************/
				if( ((GDevDataForChk[7:0] ^ DataRegA) & GDevMaskData[7:0]) && (!GangErrSckMask[0]) )
					GangChkSckStatusNext[0] = 1'b1 ; /* if socket A is enable and checking error then set flag */
				if( ((GDevDataForChk[7:0] ^ DataRegB) & GDevMaskData[7:0]) && (!GangErrSckMask[1]) )
					GangChkSckStatusNext[1] = 1'b1 ; /* if socket B is enable and checking error then set flag */
				if( ((GDevDataForChk[7:0] ^ DataRegC) & GDevMaskData[7:0]) && (!GangErrSckMask[2]) )
					GangChkSckStatusNext[2] = 1'b1 ; /* if socket C is enable and checking error then set flag */
				if( ((GDevDataForChk[7:0] ^ DataRegD) & GDevMaskData[7:0]) && (!GangErrSckMask[3]) )
					GangChkSckStatusNext[3] = 1'b1 ; /* if socket D is enable and checking error then set flag */

				SCKReg = 1'b0 ;
//				EnSCKOut = EndCnt ; /* change SCK to lo status */
				ClockStep = ClockStep + 1 ; /* re-initial wave-step to first status */
			  end
		  end

////////////////////////////////////////////////////////////////////////////////////
		else if( RdWrFlag == WriteData ) /* write data to device */
		  begin
			if( (ClockStep==0)||(ClockStep==1) )
			  begin
				SDIReg = FpgaDataReg[7] ;
				SCKReg = 1'b0 ;
			  end
			else if( (ClockStep==3)||(ClockStep==4) )
			  begin
				SDIReg = FpgaDataReg[6] ;
				SCKReg = 1'b0 ;
			  end
			else if( (ClockStep==6)||(ClockStep==7) )
			  begin
				SDIReg = FpgaDataReg[5] ;
				SCKReg = 1'b0 ;
			  end
			else if( (ClockStep==9)||(ClockStep==10) )
			  begin
				SDIReg = FpgaDataReg[4] ;
				SCKReg = 1'b0 ;
			  end
			else if( (ClockStep==12)||(ClockStep==13) )
			  begin
				SDIReg = FpgaDataReg[3] ;
				SCKReg = 1'b0 ;
			  end
			else if( (ClockStep==15)||(ClockStep==16) )
			  begin
				SDIReg = FpgaDataReg[2] ;
				SCKReg = 1'b0 ;
			  end
			else if( (ClockStep==18)||(ClockStep==19) )
			  begin
				SDIReg = FpgaDataReg[1] ;
				SCKReg = 1'b0 ;
			  end
			else if( (ClockStep==21)||(ClockStep==22) )
			  begin
				SDIReg = FpgaDataReg[0] ;
				SCKReg = 1'b0 ;
			  end
			else if( ClockStep < 24 )
				SCKReg = 1'b1 ;

	
			if( ClockStep < 24 )
				ClockStep = ClockStep + 1 ;
			else if( ClockStep == 24 )
			  begin
				SCKReg = 1'b0 ;
//				EnSCKOut = EndCnt ; /* change SCK to lo status */
				ClockStep = ClockStep + 1 ; /* re-initial wave-step to first status */
			  end
		  end
	  end

/////////////////////////////////////////////////////////////////////////
	else
	  begin
		GangChkSckStatusNext = 16'b0 ;
		SCKReg = 1'b0 ;
		ClockStep = 0 ; /* re-initial wave-step to first status */
	  end

//	SYSCLKReg = ~SYSCLKReg ;

  end


/*******************************************************************/
/*******************************************************************/
always @(posedge FPGAGCLK0) /* Scan Clock = 66MH/2 = 33MHz */
  begin /* Nor(ADM) Flash Logic Process >=15ns, So MUST use <= 33MHz Scan Clock */

	if( ClockStep > 24) /* re-initial wave-step to first status */
	  begin
		if( RdWrFlag == ReadData )
			GangChkSckStatus = GangChkSckStatus | {12'b0,GangChkSckStatusNext[3:0]} ;
		EnSCKOut = EndCnt ; /* change SCK to lo status */
	  end

	//////////////////////////////////////////////////////
	case (StateReg)
		IdleState:
		  begin
			RstReg = 1 ;

			if( FPGARD && FPGAWR )
			  begin
				StateReg = IdleState ; /* idle state */
			  end

			else
			  begin
//				SCKReg = 1'b0 ;
				CSReg = 1 ;
				StateReg = BusyState ; /* busy state */
			  end
		  end

////////////////////////////////////////////////////////
		BusyState:
		  begin
			if( !FPGARD )
			  begin
				if( FPGAAddr[15:8] < SetFpgaPara[15:8] )
				  begin
					if( (FPGAAddr[15:8]==StartReadData[15:8]) )
					  begin
						StateReg = ReadDataLo ; /* read data */
						EnSCKOut = StartCnt ;
						RdWrFlag = ReadData ;
					  end
					else if( (FPGAAddr[15:8]==OneDataCmd[15:8]) )
					  begin
						StateReg = ReadDataLo ; /* read data */
					  end

					else
					  begin
						if( FPGAAddr[15:8] < GndPinMinNum[15:8] )
							StateReg = ReadVccStatus ;
						else/* if( FPGAAddr[15:8]==GndPinMinNum[15:8] )*/
							StateReg = ReadGndStatus ;
					  end
				  end

				else
					StateReg = ReadSetting ;
			  end

		////////////////////////////////////////////////////
			else if( !FPGAWR )
			  begin
				if( FPGAAddr[15:8] < SetFpgaPara[15:8] )
				  begin
					if(	(FPGAAddr[15:8]==GDevChkDataLoInc0[15:8]) )
					  begin
						StateReg = ReadGangData ;
						EnSCKOut = StartCnt ;
						RdWrFlag = ReadData ;
					  end

					else
					  begin
						StateReg = WriteDataLo ;
						EnSCKOut = StartCnt ;
						RdWrFlag = WriteData ;
						FpgaDataReg[7:0] = FPGAData[7:0] ;
					  end
				  end

				else
					StateReg = WriteSetting ;
			  end

			else /*if( FPGARD && FPGAWR )*/
				StateReg = BusyState ; /* Busy state */
		  end

/////////////////////////////////////////////////////
		ReadDataLo:
		  begin
			if( !FPGARD )
			  begin
				if( EnSelSocketNum[0] )
					GangChkSckStatus = {8'b0,DataRegA} ;
				else if( EnSelSocketNum[1] )
					GangChkSckStatus = {8'b0,DataRegB} ;
				else if( EnSelSocketNum[2] )
					GangChkSckStatus = {8'b0,DataRegC} ;
				else if( EnSelSocketNum[3] )
					GangChkSckStatus = {8'b0,DataRegD} ;
			  end

			else
			  begin
				StateReg = BusyState ;
			  end
		  end


///////////////////////////////////////////////////////
		ReadGangData:
		  begin
			if( !FPGAWR )
			  begin
//				GangChkSckStatus = 16'b0 ; /* clear check flag */
				GDevDataForChk = {8'b0,FPGAData[7:0]} ;
			  end

			else
			  begin
				StateReg = BusyState ;
			  end
		  end


/////////////////////////////////////////////////////
		ReadSetting:
		  begin
			if( !FPGARD )
			  begin
				if( FPGAAddr[15:8]==GChkAllSckStatus[15:8] )
				  begin/* checking word data & fee-back byte result to system */

				  end

			  else if( FPGAAddr[15:8]==FPGAComChk5A5A[15:8] )
					GangChkSckStatus = 16'h5A5A ; /* check to confirm FPGA communication prot ok */
			  else if( FPGAAddr[15:8]==FPGAComChkA5A5[15:8] )
					GangChkSckStatus = 16'hA5A5 ; /* check to confirm FPGA communication prot ok */

		///////// below just for debug /////////////////////////////
//				else if( FPGAAddr[15:8]==SetAddrLoWord[15:8] )
//					GangChkSckStatus[15:0] = AddrReg[15:0] ;
//				else if( FPGAAddr[15:8]==SetAddrHiWord[15:8] )
//					GangChkSckStatus[15:0] = AddrReg[31:16] ;
//				else if( FPGAAddr[15:8]==GGetSckxData[15:8] )
//				  begin
//					if( EnSelSocketNum==EnSckACtrlFlag )
//						GangChkSckStatus = DataRegA ;
//					else if( EnSelSocketNum==EnSckBCtrlFlag )
//						GangChkSckStatus = DataRegB ;
//					else if( EnSelSocketNum==EnSckCCtrlFlag )
//						GangChkSckStatus = DataRegC ;
//					else/* if( EnSelSocketNum==EnSckDCtrlFlag )*/
//						GangChkSckStatus = DataRegD ;
//				  end
//				else if( FPGAAddr[15:8]==GSetAllErrSckMask[15:8] )
//					GangChkSckStatus = GangErrSckMask ;
//				else if( FPGAAddr[15:8]==GSetAllMaskDataLo[15:8] )
//					GangChkSckStatus = GDevMaskData ;
			  end

			else
				StateReg = BusyState ;
		  end

///////////////////////////////////////////////////
		WriteDataLo:
		  begin
			if( !FPGAWR )
			  begin
				FpgaDataReg[7:0] = FPGAData[7:0] ;
			  end

			else
			  begin
				StateReg = BusyState ;
			  end
		  end


/////////////////////////////////////////////////////
		WriteSetting:
		  begin
			if( FPGAAddr[15:8]==FPGADisAllPin[15:8] )
				StateReg = RDWRSetIdle ;

		//////////////////////////////////////////////////////////
			else
			  begin
				if( !FPGAWR )
				  begin
					if( FPGAAddr[15:8]==GClrPageChkStatus[15:8] )
						GangChkSckStatus = FPGAData ;
					else if( FPGAAddr[15:8]==SetAddrLoWord[15:8] )
						AddrReg[15:0] = FPGAData ;
					else if( FPGAAddr[15:8]==SetAddrHiWord[15:8] )
						AddrReg[31:16] = FPGAData ;
					else if( FPGAAddr[15:8]==SetRSTPinCmd[15:8] )
					  begin
						RstReg = FPGAData[0] ;
					  end
					else if( FPGAAddr[15:8]==SetCSPinCmd[15:8] )
					  begin
//						SCKReg = 1'b0 ;
						CSReg = FPGAData[0] ;
					  end
		
					else if( FPGAAddr[15:8]==GSetAllErrSckMask[15:8] )
						GangErrSckMask = FPGAData ;
					else if( FPGAAddr[15:8]==GSetAllMaskDataLo[15:8] )
						GDevMaskData = FPGAData ;
					else if( FPGAAddr[15:8]==GEnSelSckNum[15:8] )
						EnSelSocketNum = FPGAData[7:0] ;
					else if( FPGAAddr[15:8]==FPGAEnAllPin[15:8] )
					  begin
						RstReg = 1 ;
//						SCKReg = 1'b0 ;
						CSReg = 1 ;
						EnSelSocketNum = EnAllSckCtrlFlag ; /* set default to enable all socket data */
					  end
				  end

				else
					StateReg = BusyState ;
			  end
		  end

		WriteSetEnd:
		  begin
			if( FPGAWR )
				StateReg = BusyState ;
		  end

		RDWRSetIdle:
		  begin
			if( FPGAWR )
				StateReg = IdleState ;
		  end

/////////////////////////////////////////////////////
		ReadVccStatus:
		  begin
			if( !FPGARD )
			  begin
				GangChkSckStatus = 1'b1 ; /* PCC Pass */
//		/* socket A pin out */
//				if( EnSelSocketNum[0] )
//				  begin
//						GangChkSckStatus = VccPortA ; /* PCC OK */
//				  end
//
//		/* socket B pin out */
//				else if( EnSelSocketNum[1] ) /* socket B pin out */
//				  begin
//						GangChkSckStatus = VccPortB ; /* PCC OK */
//				  end
//
//		/* socket C pin out */
//				else if( EnSelSocketNum[2] ) /* socket C pin out */
//				  begin
//						GangChkSckStatus = VccPortC ; /* PCC OK */
//				  end
//
//		/* socket D pin out */
//				else if( EnSelSocketNum[3] ) /* socket D pin out */
//				  begin
//						GangChkSckStatus = VccPortD ; /* PCC OK */
//				  end
			  end

			else
			  begin
				StateReg = BusyState ;
			  end
		  end

/////////////////////////////////////////////////////
		ReadGndStatus:
		  begin
			if( !FPGARD )
			  begin
				GangChkSckStatus = 1'b1 ; /* PCC Error */
		/* socket A pin out */
				if( EnSelSocketNum[0] )
				  begin
						GangChkSckStatus = GndPortA ; /* PCC OK */
				  end

		/* socket B pin out */
				else if( EnSelSocketNum[1] )
				  begin
					GangChkSckStatus = GndPortB ; /* PCC OK */
				  end

		/* socket C pin out */
				else if( EnSelSocketNum[2] )
				  begin
					GangChkSckStatus = GndPortC ; /* PCC OK */
				  end

		/* socket D pin out */
				else if( EnSelSocketNum[3] )
				  begin
					GangChkSckStatus = GndPortD ; /* PCC OK */
				  end
			  end

			else
			  begin
				StateReg = BusyState ;
			  end
		  end

////////////////////////////////////////////////////////////////
		default:
			if( FPGARD && FPGAWR )
				StateReg = IdleState ;
	endcase

  end



/////////////////////////////////////////////////////////////////////////
//////////////////// Port Driving Control ///////////////////////////
assign FPGAData = ((StateReg==ReadDataLo)||(StateReg==ReadSetting)||(StateReg==ReadVccStatus)||(StateReg==ReadGndStatus)) ? GangChkSckStatus : 16'bz ;


/////////////////////////////////////////////////////////////////////////
///// Socket A signal, if checking error then skip all signal ///////////
/////////////////////////////////////////////////////////////////////////
assign SCKPortA = ((StateReg==IdleState)||(GangErrSckMask[0])) ? 1'bz : SCKReg ;
assign SDIPortA = ((StateReg==IdleState)||(GangErrSckMask[0])) ? 1'bz : SDIReg ; /* write data to chip */
assign CSPortA = (StateReg==IdleState) ? 1'bz : ((GangErrSckMask[0]||(!EnSelSocketNum[0])) ? 1'b1 : CSReg) ; /* when error disable device! */
assign HoldPortA = ((StateReg==IdleState)||(GangErrSckMask[0])) ? 1'bz : 1'b1 ;
assign WPPortA = ((StateReg==IdleState)||(GangErrSckMask[0])) ? 1'bz : 1'b1 ;

assign GndPortA = 1'bz ; /* setting Gnd/Vcc Pin to Z-state to protect FPGA */
assign VccPortA = 1'bz ; /* these two signal control by DAC board */


/////////////////////////////////////////////////////////////////////////
///// Socket B signal, if checking error then skip all signal ///////////
/////////////////////////////////////////////////////////////////////////
assign SCKPortB = ((StateReg==IdleState)||(GangErrSckMask[1])) ? 1'bz : SCKReg ;
assign SDIPortB = ((StateReg==IdleState)||(GangErrSckMask[1])) ? 1'bz : SDIReg ; /* write data to chip */
assign CSPortB = (StateReg==IdleState) ? 1'bz : ((GangErrSckMask[1]||(!EnSelSocketNum[1])) ? 1'b1 : CSReg) ; /* when error disable device! */
assign HoldPortB = ((StateReg==IdleState)||(GangErrSckMask[1])) ? 1'bz : 1'b1 ;
assign WPPortB = ((StateReg==IdleState)||(GangErrSckMask[1])) ? 1'bz : 1'b1 ;

assign GndPortB = 1'bz ; /* setting Gnd/Vcc Pin to Z-state to protect FPGA */
assign VccPortB = 1'bz ; /* these two signal control by DAC board */


/////////////////////////////////////////////////////////////////////////
///// Socket C signal, if checking error then skip all signal ///////////
/////////////////////////////////////////////////////////////////////////
assign SCKPortC = ((StateReg==IdleState)||(GangErrSckMask[2])) ? 1'bz : SCKReg ;
assign SDIPortC = ((StateReg==IdleState)||(GangErrSckMask[2])) ? 1'bz : SDIReg ; /* write data to chip */
assign CSPortC = (StateReg==IdleState) ? 1'bz : ((GangErrSckMask[2]||(!EnSelSocketNum[2])) ? 1'b1 : CSReg) ; /* when error disable device! */
assign HoldPortC = ((StateReg==IdleState)||(GangErrSckMask[2])) ? 1'bz : 1'b1 ;
assign WPPortC = ((StateReg==IdleState)||(GangErrSckMask[2])) ? 1'bz : 1'b1 ;

assign GndPortC = 1'bz ; /* setting Gnd/Vcc Pin to Z-state to protect FPGA */
assign VccPortC = 1'bz ; /* these two signal control by DAC board */


/////////////////////////////////////////////////////////////////////////
///// Socket D signal, if checking error then skip all signal ///////////
/////////////////////////////////////////////////////////////////////////
assign SCKPortD = ((StateReg==IdleState)||(GangErrSckMask[3])) ? 1'bz : SCKReg ;
assign SDIPortD = ((StateReg==IdleState)||(GangErrSckMask[3])) ? 1'bz : SDIReg ; /* write data to chip */
assign CSPortD = (StateReg==IdleState) ? 1'bz : ((GangErrSckMask[3]||(!EnSelSocketNum[3])) ? 1'b1 : CSReg) ; /* when error disable device! */
assign HoldPortD = ((StateReg==IdleState)||(GangErrSckMask[3])) ? 1'bz : 1'b1 ;
assign WPPortD = ((StateReg==IdleState)||(GangErrSckMask[3])) ? 1'bz : 1'b1 ;

assign GndPortD = 1'bz ; /* setting Gnd/Vcc Pin to Z-state to protect FPGA */
assign VccPortD = 1'bz ; /* these two signal control by DAC board */


endmodule		