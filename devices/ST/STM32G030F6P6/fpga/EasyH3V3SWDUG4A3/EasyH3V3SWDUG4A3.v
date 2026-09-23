
module	EasyH3V3SWDUG4A3(
					FPGAGCLK0,FPGAGCLK1,
					DioPort,
					FPGAAddr,
					FPGAData,
					FPGAWR,FPGARD	) ;


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
inout[192:1] DioPort;
reg[48:1] DioPortReg ; /* pin data */
reg[48:1] DioPortRW ; /* pin dir, in=0, out=1 */
reg[48:1] DioDisStatus ; /* pin status when disable this sck, 0=Lo, 1=Hi */

reg [7:0] VCCPinNumReg ;
reg [7:0] GNDPinNumReg ;
reg [7:0] SWDPinNumReg ;
reg [7:0] SCKPinNumReg ;
reg [7:0] RSTPinNumReg ;
reg [7:0] TMSPinNumReg ;
reg [7:0] TDIPinNumReg ;
reg [7:0] JCOMPinNumReg ;



/********************************************************************/
/*            Socket A signal                                       */
/********************************************************************/


/********************************************************************/
/*            Socket B signal                                       */
/********************************************************************/


/********************************************************************/
/*            Socket C signal                                       */
/********************************************************************/


/********************************************************************/
/*            Socket D signal                                       */
/********************************************************************/



/********************************************************************/
/*            system register                                       */
/********************************************************************/
reg [5:0] StateReg ;
reg [15:0] FpgaDataReg ;
reg [15:0] DataRegA ;
reg [15:0] DataRegB ;
reg [15:0] DataRegC ;
reg [15:0] DataRegD ;
reg [31:0] AddrReg, AddrBackReg ;



/********************************************************************/
/*		user define register here                                   */
/********************************************************************/
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
parameter	SetSCKPinCmd		= 16'h8200 ;
parameter	SetCSPinCmd			= 16'h8300 ;
parameter	SetCLEPinCmd		= 16'h8400 ;
parameter	SetALEPinCmd		= 16'h8500 ;
parameter	SetTMSPinCmd		= 16'h8600 ;
parameter	SetTDIPinCmd		= 16'h8700 ;
parameter	SetJCOMPinCmd		= 16'h8800 ;

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

///////////////////////////////////////////////////////////////
parameter	SetVCCPinNum		= 16'hB000 ; /* 0xB000 for pin control */
parameter	SetGNDPinNum		= 16'hB100 ;
parameter	SetSWDPinNum		= 16'hB200 ; /* can also be TDO for JTAG */
parameter	SetSCKPinNum		= 16'hB300 ; /* can also be TCK for JTAG */
parameter	SetRSTPinNum		= 16'hB400 ;
parameter	SetTMSPinNum		= 16'hB500 ;
parameter	SetTDIPinNum		= 16'hB600 ;
parameter	SetJCOMPinNum		= 16'hB700 ;


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
	EnSelSocketNum <= EnAllSckCtrlFlag ; /* set default to read socket A data */

	DioPortRW[48:1] <= 48'b0 ; /* set all 48 pins be input!! */
	StateReg <= IdleState ; /* MUST define together */
	AddrReg <= 32'b0 ;
	AddrBackReg <= 32'b0 ;

	ClockStep <= 8'b0 ; /* re-initial wave-step to first status */
  end


/*******************************************************************\
\*******************************************************************/
always @(posedge FPGAGCLK0) /* Scan Clock = 66MH/2 = 33MHz */
  begin /* Nor(ADM) Flash Logic Process >=15ns, So MUST use <= 33MHz Scan Clock */
	//////////////////////////////////////////////////////
	case (StateReg)
		IdleState:
		  begin
			if( FPGARD && FPGAWR )
			  begin
				StateReg = IdleState ; /* idle state */
			  end

			else
			  begin
				StateReg = BusyState ; /* busy state */
			  end
		  end

////////////////////////////////////////////////////////
		BusyState:
		  begin
			if( !FPGARD )
			  begin
				if( !FPGAAddr[15] )
				  begin
					if( (FPGAAddr[15:8]==OneDataCmd[15:8]) )
					  begin
						StateReg = ReadDataLo ; /* read data */
		  			DioPortRW[SWDPinNumReg] = 0 ; /* Change A,B,C,D to input status */
					  end
					else if( FPGAAddr[15:8]==GndPinMinNum[15:8] )
					  begin
						StateReg = ReadGndStatus ;
		  			DioPortRW[GNDPinNumReg] = 0 ; /* Change A,B,C,D to input status */
					  end

					else
						StateReg = BusyState ;
				  end

				else
					StateReg = ReadSetting ;
			  end

		////////////////////////////////////////////////////
			else if( !FPGAWR )
			  begin
				if( !FPGAAddr[15] )
				  begin
					if(	(FPGAAddr[15:8]==GDevChkDataLoInc0[15:8]) )
					  begin
						StateReg = ReadGangData ;
		  			DioPortRW[SWDPinNumReg] = 0 ; /* Change A,B,C,D to input status */
					  end

					else
					  begin
						StateReg = WriteDataLo ;
		  			DioPortRW[SWDPinNumReg] = 1 ; /* Change A,B,C,D to output status */
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
  			DioPortRW[SWDPinNumReg] = 0 ; /* Change A,B,C,D to input status */

				if( EnSelSocketNum[0] )
					GangChkSckStatus = {15'b0,DioPort[SWDPinNumReg]} ;
				else if( EnSelSocketNum[1] )
					GangChkSckStatus = {15'b0,DioPort[SWDPinNumReg+48]} ;
				else if( EnSelSocketNum[2] )
					GangChkSckStatus = {15'b0,DioPort[SWDPinNumReg+96]} ;
				else if( EnSelSocketNum[3] )
					GangChkSckStatus = {15'b0,DioPort[SWDPinNumReg+144]} ;
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
				DioPortRW[SWDPinNumReg] = 0 ; /* Change A,B,C,D to input status */
				GDevDataForChk <= FPGAData ;
				DataRegA[ClockStep] <= {15'b0,DioPort[SWDPinNumReg]} ;
				DataRegB[ClockStep] <= {15'b0,DioPort[SWDPinNumReg+48]} ;
				DataRegC[ClockStep] <= {15'b0,DioPort[SWDPinNumReg+96]} ;
				DataRegD[ClockStep] <= {15'b0,DioPort[SWDPinNumReg+144]} ;
			  end

			else
			  begin
				ClockStep = ClockStep + 1 ;
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
					/********* Check 4 Socket data after read finish ******************/
					if( ((GDevDataForChk ^ DataRegA) & GDevMaskData) && (!GangErrSckMask[0]) )
						GangChkSckStatus[0] = 1'b1 ; /* if socket A is enable and checking error then set flag */
					if( ((GDevDataForChk ^ DataRegB) & GDevMaskData) && (!GangErrSckMask[1]) )
						GangChkSckStatus[1] = 1'b1 ; /* if socket B is enable and checking error then set flag */
					if( ((GDevDataForChk ^ DataRegC) & GDevMaskData) && (!GangErrSckMask[2]) )
						GangChkSckStatus[2] = 1'b1 ; /* if socket C is enable and checking error then set flag */
					if( ((GDevDataForChk ^ DataRegD) & GDevMaskData) && (!GangErrSckMask[3]) )
						GangChkSckStatus[3] = 1'b1 ; /* if socket D is enable and checking error then set flag */
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
				DioPortRW[SWDPinNumReg] = 1 ; /* Change A,B,C,D to output status */
				DioPortReg[SWDPinNumReg] = FPGAData[0] ;
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
			  begin
				StateReg = RDWRSetIdle ;
				DioPortRW[48:1] = 48'b0 ; /* set all pin to Z-state */
				DioPortReg[48:1] = 48'b0 ;
			  end

		//////////////////////////////////////////////////////////
			else
			  begin
				if( !FPGAWR )
				  begin
					if( FPGAAddr[15:8]==GClrPageChkStatus[15:8] )
					  begin
						GangChkSckStatus = FPGAData ;
						ClockStep = 8'h00 ; /* 16 bit mode */
					  end
					else if( FPGAAddr[15:8]==SetAddrLoWord[15:8] )
						AddrReg[15:0] = FPGAData ;
					else if( FPGAAddr[15:8]==SetAddrHiWord[15:8] )
						AddrReg[31:16] = FPGAData ;

					else if( (FPGAAddr[15:8]==SetSCKPinCmd[15:8]) )
					  begin
						DioPortRW[SCKPinNumReg] = 1 ; /* Change A,B,C,D to output status */
						DioPortReg[SCKPinNumReg] = FPGAData[0] ;
					  end
					else if( (FPGAAddr[15:8]==SetRSTPinCmd[15:8]) )
					  begin
						DioPortRW[RSTPinNumReg] = 1 ; /* Change A,B,C,D to output status */
						DioPortReg[RSTPinNumReg] = FPGAData[0] ;
					  end
					else if( (FPGAAddr[15:8]==SetTMSPinCmd[15:8]) )
					  begin
						DioPortRW[TMSPinNumReg] = 1 ; /* Change A,B,C,D to output status */
						DioPortReg[TMSPinNumReg] = FPGAData[0] ;
						end
					else if( (FPGAAddr[15:8]==SetTDIPinCmd[15:8]) )
					  begin
						DioPortRW[TDIPinNumReg] = 1 ; /* Change A,B,C,D to output status */
						DioPortReg[TDIPinNumReg] = FPGAData[0] ;
						end
					else if( (FPGAAddr[15:8]==SetJCOMPinCmd[15:8]) )
					  begin
						DioPortRW[JCOMPinNumReg] = 1 ; /* Change A,B,C,D to output status */
						DioPortReg[JCOMPinNumReg] = FPGAData[0] ;
						end

					///////////// for UNI Easy Pin FPGA define /////////
					else if( (FPGAAddr[15:8]==SetVCCPinNum[15:8]) )
					  begin
						VCCPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[VCCPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[VCCPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end
					else if( (FPGAAddr[15:8]==SetGNDPinNum[15:8]) )
					  begin
						GNDPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[GNDPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[GNDPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end
					else if( (FPGAAddr[15:8]==SetSWDPinNum[15:8]) )
					  begin
						SWDPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[SWDPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[SWDPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end
					else if( (FPGAAddr[15:8]==SetSCKPinNum[15:8]) )
					  begin
						SCKPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[SCKPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[SCKPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end
					else if( (FPGAAddr[15:8]==SetRSTPinNum[15:8]) )
					  begin
						RSTPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[RSTPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[RSTPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end
					else if( (FPGAAddr[15:8]==SetTMSPinNum[15:8]) )
					  begin
						TMSPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[TMSPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[TMSPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end
					else if( (FPGAAddr[15:8]==SetTDIPinNum[15:8]) )
					  begin
						TDIPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[TDIPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[TDIPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end
					else if( (FPGAAddr[15:8]==SetJCOMPinNum[15:8]) )
					  begin
						JCOMPinNumReg = FPGAData[7:0] ;
		  			DioPortRW[JCOMPinNumReg] = FPGAData[12] ; /* set A,B,C,D to 1=out,0=in status */
		  			DioDisStatus[JCOMPinNumReg] = FPGAData[8] ; /* set pin status when disable */
					  end

/////////////////////////////////////////////////////////////////
					else if( FPGAAddr[15:8]==GSetAllErrSckMask[15:8] )
						GangErrSckMask = FPGAData ;
					else if( FPGAAddr[15:8]==GSetAllMaskDataLo[15:8] )
						GDevMaskData = FPGAData ;
					else if( FPGAAddr[15:8]==GEnSelSckNum[15:8] )
						EnSelSocketNum = FPGAData[7:0] ;
					else if( FPGAAddr[15:8]==FPGAEnAllPin[15:8] )
					  begin
						EnSelSocketNum = EnAllSckCtrlFlag ; /* set default to enable all socket data */
//					  DioPortRW[48:1] = 48'b0 ; /* set all pin input status */
//						DioPortReg[48:1] = 48'b0 ;
					  end
				  end

				else
					StateReg = BusyState ;
			  end
		  end

	//////////////////////////////////////////////////////////////////
		RDWRSetIdle:
		  begin
			if( FPGAWR )
				StateReg = IdleState ;
		  end

/////////////////////////////////////////////////////
//		ReadVccStatus:
//		  begin
//			if( !FPGARD )
//			  begin
////				GangChkSckStatus = 1'b0 ; /* PCC Error */
//				GangChkSckStatus = 1'b1 ; /* PCC Pass */
////		/* socket A pin out */
////				if( FPGAAddr[7:0] == VCCPinA )
////				  begin
////					if( (VccPortA==1'b1) ) /* MUST all be Vih, use AND algo to check */
////						GangChkSckStatus = 1'b1 ; /* PCC OK */
////				  end
////
////		/* socket B pin out */
////				else if( FPGAAddr[7:0] == VCCPinB ) /* socket B pin out */
////				  begin
////					if( (VccPortB==1'b1) ) /* MUST all be Vih, use AND algo to check */
////						GangChkSckStatus = 1'b1 ; /* PCC OK */
////				  end
////
////		/* socket C pin out */
////				else if( FPGAAddr[7:0] == VCCPinC ) /* socket C pin out */
////				  begin
////					if( (VccPortC==1'b1) ) /* MUST all be Vih, use AND algo to check */
////						GangChkSckStatus = 1'b1 ; /* PCC OK */
////				  end
////
////		/* socket D pin out */
////				else if( FPGAAddr[7:0] == VCCPinD ) /* socket D pin out */
////				  begin
////					if( (VccPortD==1'b1) ) /* MUST all be Vih, use AND algo to check */
////						GangChkSckStatus = 1'b1 ; /* PCC OK */
////				  end
//			  end
//
//			else
//			  begin
//				StateReg = BusyState ;
//			  end
//		  end

/////////////////////////////////////////////////////
		ReadGndStatus:
		  begin
			if( !FPGARD )
			  begin
				GangChkSckStatus = 1'b1 ; /* PCC Error */
				DioPortRW[GNDPinNumReg] = 0 ; /* Change A,B,C,D to input status */
		/* socket A pin out */
				if( EnSelSocketNum[0] )
				  begin
						GangChkSckStatus = {15'b0,DioPort[GNDPinNumReg]} ; /* PCC OK */
				  end

		/* socket B pin out */
				else if( EnSelSocketNum[1] )
				  begin
					GangChkSckStatus = {15'b0,DioPort[GNDPinNumReg+48]} ; /* PCC OK */
				  end

		/* socket C pin out */
				else if( EnSelSocketNum[2] )
				  begin
					GangChkSckStatus = {15'b0,DioPort[GNDPinNumReg+96]} ; /* PCC OK */
				  end

		/* socket D pin out */
				else if( EnSelSocketNum[3] )
				  begin
					GangChkSckStatus = {15'b0,DioPort[GNDPinNumReg+144]} ; /* PCC OK */
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
assign FPGAData = ((StateReg==ReadDataLo)||(StateReg==ReadSetting)||(StateReg==ReadGndStatus)) ? GangChkSckStatus : 16'bz ;



//////////////////////////////////////////////////////////////////////////////
//////// Socket A Pin Number //////////////
assign DioPort[1] = (GangErrSckMask[0]||(!DioPortRW[1]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[1] : DioPortReg[1]) ;
assign DioPort[2] = (GangErrSckMask[0]||(!DioPortRW[2]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[2] : DioPortReg[2]) ;
assign DioPort[3] = (GangErrSckMask[0]||(!DioPortRW[3]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[3] : DioPortReg[3]) ;
assign DioPort[4] = (GangErrSckMask[0]||(!DioPortRW[4]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[4] : DioPortReg[4]) ;
assign DioPort[5] = (GangErrSckMask[0]||(!DioPortRW[5]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[5] : DioPortReg[5]) ;
assign DioPort[6] = (GangErrSckMask[0]||(!DioPortRW[6]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[6] : DioPortReg[6]) ;
assign DioPort[7] = (GangErrSckMask[0]||(!DioPortRW[7]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[7] : DioPortReg[7]) ;
assign DioPort[8] = (GangErrSckMask[0]||(!DioPortRW[8]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[8] : DioPortReg[8]) ;

assign DioPort[9]  = (GangErrSckMask[0]||(!DioPortRW[9])) ? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[9]  : DioPortReg[9]) ;
assign DioPort[10] = (GangErrSckMask[0]||(!DioPortRW[10]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[10] : DioPortReg[10]) ;
assign DioPort[11] = (GangErrSckMask[0]||(!DioPortRW[11]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[11] : DioPortReg[11]) ;
assign DioPort[12] = (GangErrSckMask[0]||(!DioPortRW[12]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[12] : DioPortReg[12]) ;
assign DioPort[13] = (GangErrSckMask[0]||(!DioPortRW[13]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[13] : DioPortReg[13]) ;
assign DioPort[14] = (GangErrSckMask[0]||(!DioPortRW[14]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[14] : DioPortReg[14]) ;
assign DioPort[15] = (GangErrSckMask[0]||(!DioPortRW[15]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[15] : DioPortReg[15]) ;
assign DioPort[16] = (GangErrSckMask[0]||(!DioPortRW[16]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[16] : DioPortReg[16]) ;

assign DioPort[17] = (GangErrSckMask[0]||(!DioPortRW[17]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[17] : DioPortReg[17]) ; 
assign DioPort[18] = (GangErrSckMask[0]||(!DioPortRW[18]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[18] : DioPortReg[18]) ;
assign DioPort[19] = (GangErrSckMask[0]||(!DioPortRW[19]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[19] : DioPortReg[19]) ;
assign DioPort[20] = (GangErrSckMask[0]||(!DioPortRW[20]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[20] : DioPortReg[20]) ;
assign DioPort[21] = (GangErrSckMask[0]||(!DioPortRW[21]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[21] : DioPortReg[21]) ;
assign DioPort[22] = (GangErrSckMask[0]||(!DioPortRW[22]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[22] : DioPortReg[22]) ;
assign DioPort[23] = (GangErrSckMask[0]||(!DioPortRW[23]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[23] : DioPortReg[23]) ;
assign DioPort[24] = (GangErrSckMask[0]||(!DioPortRW[24]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[24] : DioPortReg[24]) ;

assign DioPort[25] = (GangErrSckMask[0]||(!DioPortRW[25]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[25] : DioPortReg[25]) ;
assign DioPort[26] = (GangErrSckMask[0]||(!DioPortRW[26]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[26] : DioPortReg[26]) ;
assign DioPort[27] = (GangErrSckMask[0]||(!DioPortRW[27]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[27] : DioPortReg[27]) ;
assign DioPort[28] = (GangErrSckMask[0]||(!DioPortRW[28]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[28] : DioPortReg[28]) ;
assign DioPort[29] = (GangErrSckMask[0]||(!DioPortRW[29]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[29] : DioPortReg[29]) ;
assign DioPort[30] = (GangErrSckMask[0]||(!DioPortRW[30]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[30] : DioPortReg[30]) ;
assign DioPort[31] = (GangErrSckMask[0]||(!DioPortRW[31]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[31] : DioPortReg[31]) ;
assign DioPort[32] = (GangErrSckMask[0]||(!DioPortRW[32]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[32] : DioPortReg[32]) ;

assign DioPort[33] = (GangErrSckMask[0]||(!DioPortRW[33]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[33] : DioPortReg[33]) ;
assign DioPort[34] = (GangErrSckMask[0]||(!DioPortRW[34]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[34] : DioPortReg[34]) ;
assign DioPort[35] = (GangErrSckMask[0]||(!DioPortRW[35]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[35] : DioPortReg[35]) ;
assign DioPort[36] = (GangErrSckMask[0]||(!DioPortRW[36]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[36] : DioPortReg[36]) ;
assign DioPort[37] = (GangErrSckMask[0]||(!DioPortRW[37]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[37] : DioPortReg[37]) ;
assign DioPort[38] = (GangErrSckMask[0]||(!DioPortRW[38]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[38] : DioPortReg[38]) ;
assign DioPort[39] = (GangErrSckMask[0]||(!DioPortRW[39]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[39] : DioPortReg[39]) ;
assign DioPort[40] = (GangErrSckMask[0]||(!DioPortRW[40]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[40] : DioPortReg[40]) ;

assign DioPort[41] = (GangErrSckMask[0]||(!DioPortRW[41]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[41] : DioPortReg[41]) ;
assign DioPort[42] = (GangErrSckMask[0]||(!DioPortRW[42]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[42] : DioPortReg[42]) ;
assign DioPort[43] = (GangErrSckMask[0]||(!DioPortRW[43]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[43] : DioPortReg[43]) ;
assign DioPort[44] = (GangErrSckMask[0]||(!DioPortRW[44]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[44] : DioPortReg[44]) ;
assign DioPort[45] = (GangErrSckMask[0]||(!DioPortRW[45]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[45] : DioPortReg[45]) ;
assign DioPort[46] = (GangErrSckMask[0]||(!DioPortRW[46]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[46] : DioPortReg[46]) ;
assign DioPort[47] = (GangErrSckMask[0]||(!DioPortRW[47]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[47] : DioPortReg[47]) ;
assign DioPort[48] = (GangErrSckMask[0]||(!DioPortRW[48]))? 1'bz :((!EnSelSocketNum[0]) ? DioDisStatus[48] : DioPortReg[48]) ;
       
//////////////////////////////////////////////////////////////////////////////              
//////// Socket B Pin Number //////////////
assign DioPort[49] = (GangErrSckMask[1]||(!DioPortRW[1]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[1] : DioPortReg[1]) ;
assign DioPort[50] = (GangErrSckMask[1]||(!DioPortRW[2]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[2] : DioPortReg[2]) ;
assign DioPort[51] = (GangErrSckMask[1]||(!DioPortRW[3]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[3] : DioPortReg[3]) ;
assign DioPort[52] = (GangErrSckMask[1]||(!DioPortRW[4]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[4] : DioPortReg[4]) ;
assign DioPort[53] = (GangErrSckMask[1]||(!DioPortRW[5]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[5] : DioPortReg[5]) ;
assign DioPort[54] = (GangErrSckMask[1]||(!DioPortRW[6]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[6] : DioPortReg[6]) ;
assign DioPort[55] = (GangErrSckMask[1]||(!DioPortRW[7]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[7] : DioPortReg[7]) ;
assign DioPort[56] = (GangErrSckMask[1]||(!DioPortRW[8]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[8] : DioPortReg[8]) ;

assign DioPort[57] = (GangErrSckMask[1]||(!DioPortRW[9])) ? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[9]  : DioPortReg[9]) ; 
assign DioPort[58] = (GangErrSckMask[1]||(!DioPortRW[10]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[10] : DioPortReg[10]) ;
assign DioPort[59] = (GangErrSckMask[1]||(!DioPortRW[11]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[11] : DioPortReg[11]) ;
assign DioPort[60] = (GangErrSckMask[1]||(!DioPortRW[12]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[12] : DioPortReg[12]) ;
assign DioPort[61] = (GangErrSckMask[1]||(!DioPortRW[13]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[13] : DioPortReg[13]) ;
assign DioPort[62] = (GangErrSckMask[1]||(!DioPortRW[14]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[14] : DioPortReg[14]) ;
assign DioPort[63] = (GangErrSckMask[1]||(!DioPortRW[15]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[15] : DioPortReg[15]) ;
assign DioPort[64] = (GangErrSckMask[1]||(!DioPortRW[16]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[16] : DioPortReg[16]) ;
                                                                                                                              
assign DioPort[65] = (GangErrSckMask[1]||(!DioPortRW[17]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[17] : DioPortReg[17]) ;
assign DioPort[66] = (GangErrSckMask[1]||(!DioPortRW[18]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[18] : DioPortReg[18]) ;
assign DioPort[67] = (GangErrSckMask[1]||(!DioPortRW[19]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[19] : DioPortReg[19]) ;
assign DioPort[68] = (GangErrSckMask[1]||(!DioPortRW[20]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[20] : DioPortReg[20]) ;
assign DioPort[69] = (GangErrSckMask[1]||(!DioPortRW[21]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[21] : DioPortReg[21]) ;
assign DioPort[70] = (GangErrSckMask[1]||(!DioPortRW[22]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[22] : DioPortReg[22]) ;
assign DioPort[71] = (GangErrSckMask[1]||(!DioPortRW[23]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[23] : DioPortReg[23]) ;
assign DioPort[72] = (GangErrSckMask[1]||(!DioPortRW[24]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[24] : DioPortReg[24]) ;
                                                                                                                              
assign DioPort[73] = (GangErrSckMask[1]||(!DioPortRW[25]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[25] : DioPortReg[25]) ;
assign DioPort[74] = (GangErrSckMask[1]||(!DioPortRW[26]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[26] : DioPortReg[26]) ;
assign DioPort[75] = (GangErrSckMask[1]||(!DioPortRW[27]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[27] : DioPortReg[27]) ;
assign DioPort[76] = (GangErrSckMask[1]||(!DioPortRW[28]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[28] : DioPortReg[28]) ;
assign DioPort[77] = (GangErrSckMask[1]||(!DioPortRW[29]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[29] : DioPortReg[29]) ;
assign DioPort[78] = (GangErrSckMask[1]||(!DioPortRW[30]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[30] : DioPortReg[30]) ;
assign DioPort[79] = (GangErrSckMask[1]||(!DioPortRW[31]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[31] : DioPortReg[31]) ;
assign DioPort[80] = (GangErrSckMask[1]||(!DioPortRW[32]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[32] : DioPortReg[32]) ;
                                                                                                                              
assign DioPort[81] = (GangErrSckMask[1]||(!DioPortRW[33]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[33] : DioPortReg[33]) ;
assign DioPort[82] = (GangErrSckMask[1]||(!DioPortRW[34]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[34] : DioPortReg[34]) ;
assign DioPort[83] = (GangErrSckMask[1]||(!DioPortRW[35]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[35] : DioPortReg[35]) ;
assign DioPort[84] = (GangErrSckMask[1]||(!DioPortRW[36]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[36] : DioPortReg[36]) ;
assign DioPort[85] = (GangErrSckMask[1]||(!DioPortRW[37]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[37] : DioPortReg[37]) ;
assign DioPort[86] = (GangErrSckMask[1]||(!DioPortRW[38]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[38] : DioPortReg[38]) ;
assign DioPort[87] = (GangErrSckMask[1]||(!DioPortRW[39]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[39] : DioPortReg[39]) ;
assign DioPort[88] = (GangErrSckMask[1]||(!DioPortRW[40]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[40] : DioPortReg[40]) ;
                                                                                                                              
assign DioPort[89] = (GangErrSckMask[1]||(!DioPortRW[41]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[41] : DioPortReg[41]) ;
assign DioPort[90] = (GangErrSckMask[1]||(!DioPortRW[42]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[42] : DioPortReg[42]) ;
assign DioPort[91] = (GangErrSckMask[1]||(!DioPortRW[43]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[43] : DioPortReg[43]) ;
assign DioPort[92] = (GangErrSckMask[1]||(!DioPortRW[44]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[44] : DioPortReg[44]) ;
assign DioPort[93] = (GangErrSckMask[1]||(!DioPortRW[45]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[45] : DioPortReg[45]) ;
assign DioPort[94] = (GangErrSckMask[1]||(!DioPortRW[46]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[46] : DioPortReg[46]) ;
assign DioPort[95] = (GangErrSckMask[1]||(!DioPortRW[47]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[47] : DioPortReg[47]) ;
assign DioPort[96] = (GangErrSckMask[1]||(!DioPortRW[48]))? 1'bz :((!EnSelSocketNum[1]) ? DioDisStatus[48] : DioPortReg[48]) ;
                
//////////////////////////////////////////////////////////////////////////////
//////// Socket C Pin Number //////////////
assign DioPort[97]  = (GangErrSckMask[2]||(!DioPortRW[1]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[1] : DioPortReg[1]) ;    
assign DioPort[98]  = (GangErrSckMask[2]||(!DioPortRW[2]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[2] : DioPortReg[2]) ;    
assign DioPort[99]  = (GangErrSckMask[2]||(!DioPortRW[3]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[3] : DioPortReg[3]) ;    
assign DioPort[100] = (GangErrSckMask[2]||(!DioPortRW[4]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[4] : DioPortReg[4]) ;    
assign DioPort[101] = (GangErrSckMask[2]||(!DioPortRW[5]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[5] : DioPortReg[5]) ;    
assign DioPort[102] = (GangErrSckMask[2]||(!DioPortRW[6]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[6] : DioPortReg[6]) ;    
assign DioPort[103] = (GangErrSckMask[2]||(!DioPortRW[7]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[7] : DioPortReg[7]) ;    
assign DioPort[104] = (GangErrSckMask[2]||(!DioPortRW[8]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[8] : DioPortReg[8]) ;    
                                                                                                                                
assign DioPort[105] = (GangErrSckMask[2]||(!DioPortRW[9])) ? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[9]  : DioPortReg[9]) ;  
assign DioPort[106] = (GangErrSckMask[2]||(!DioPortRW[10]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[10] : DioPortReg[10]) ; 
assign DioPort[107] = (GangErrSckMask[2]||(!DioPortRW[11]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[11] : DioPortReg[11]) ; 
assign DioPort[108] = (GangErrSckMask[2]||(!DioPortRW[12]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[12] : DioPortReg[12]) ; 
assign DioPort[109] = (GangErrSckMask[2]||(!DioPortRW[13]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[13] : DioPortReg[13]) ; 
assign DioPort[110] = (GangErrSckMask[2]||(!DioPortRW[14]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[14] : DioPortReg[14]) ; 
assign DioPort[111] = (GangErrSckMask[2]||(!DioPortRW[15]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[15] : DioPortReg[15]) ; 
assign DioPort[112] = (GangErrSckMask[2]||(!DioPortRW[16]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[16] : DioPortReg[16]) ; 
                                                                                                                                
assign DioPort[113] = (GangErrSckMask[2]||(!DioPortRW[17]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[17] : DioPortReg[17]) ; 
assign DioPort[114] = (GangErrSckMask[2]||(!DioPortRW[18]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[18] : DioPortReg[18]) ; 
assign DioPort[115] = (GangErrSckMask[2]||(!DioPortRW[19]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[19] : DioPortReg[19]) ; 
assign DioPort[116] = (GangErrSckMask[2]||(!DioPortRW[20]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[20] : DioPortReg[20]) ; 
assign DioPort[117] = (GangErrSckMask[2]||(!DioPortRW[21]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[21] : DioPortReg[21]) ; 
assign DioPort[118] = (GangErrSckMask[2]||(!DioPortRW[22]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[22] : DioPortReg[22]) ; 
assign DioPort[119] = (GangErrSckMask[2]||(!DioPortRW[23]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[23] : DioPortReg[23]) ; 
assign DioPort[120] = (GangErrSckMask[2]||(!DioPortRW[24]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[24] : DioPortReg[24]) ; 
                                                                                                                                
assign DioPort[121] = (GangErrSckMask[2]||(!DioPortRW[25]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[25] : DioPortReg[25]) ; 
assign DioPort[122] = (GangErrSckMask[2]||(!DioPortRW[26]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[26] : DioPortReg[26]) ; 
assign DioPort[123] = (GangErrSckMask[2]||(!DioPortRW[27]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[27] : DioPortReg[27]) ; 
assign DioPort[124] = (GangErrSckMask[2]||(!DioPortRW[28]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[28] : DioPortReg[28]) ; 
assign DioPort[125] = (GangErrSckMask[2]||(!DioPortRW[29]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[29] : DioPortReg[29]) ; 
assign DioPort[126] = (GangErrSckMask[2]||(!DioPortRW[30]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[30] : DioPortReg[30]) ; 
assign DioPort[127] = (GangErrSckMask[2]||(!DioPortRW[31]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[31] : DioPortReg[31]) ; 
assign DioPort[128] = (GangErrSckMask[2]||(!DioPortRW[32]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[32] : DioPortReg[32]) ; 
                                                                                                                                
assign DioPort[129] = (GangErrSckMask[2]||(!DioPortRW[33]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[33] : DioPortReg[33]) ; 
assign DioPort[130] = (GangErrSckMask[2]||(!DioPortRW[34]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[34] : DioPortReg[34]) ; 
assign DioPort[131] = (GangErrSckMask[2]||(!DioPortRW[35]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[35] : DioPortReg[35]) ; 
assign DioPort[132] = (GangErrSckMask[2]||(!DioPortRW[36]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[36] : DioPortReg[36]) ; 
assign DioPort[133] = (GangErrSckMask[2]||(!DioPortRW[37]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[37] : DioPortReg[37]) ; 
assign DioPort[134] = (GangErrSckMask[2]||(!DioPortRW[38]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[38] : DioPortReg[38]) ; 
assign DioPort[135] = (GangErrSckMask[2]||(!DioPortRW[39]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[39] : DioPortReg[39]) ; 
assign DioPort[136] = (GangErrSckMask[2]||(!DioPortRW[40]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[40] : DioPortReg[40]) ; 
                                                                                                                                
assign DioPort[137] = (GangErrSckMask[2]||(!DioPortRW[41]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[41] : DioPortReg[41]) ; 
assign DioPort[138] = (GangErrSckMask[2]||(!DioPortRW[42]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[42] : DioPortReg[42]) ; 
assign DioPort[139] = (GangErrSckMask[2]||(!DioPortRW[43]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[43] : DioPortReg[43]) ; 
assign DioPort[140] = (GangErrSckMask[2]||(!DioPortRW[44]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[44] : DioPortReg[44]) ; 
assign DioPort[141] = (GangErrSckMask[2]||(!DioPortRW[45]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[45] : DioPortReg[45]) ; 
assign DioPort[142] = (GangErrSckMask[2]||(!DioPortRW[46]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[46] : DioPortReg[46]) ; 
assign DioPort[143] = (GangErrSckMask[2]||(!DioPortRW[47]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[47] : DioPortReg[47]) ; 
assign DioPort[144] = (GangErrSckMask[2]||(!DioPortRW[48]))? 1'bz :((!EnSelSocketNum[2]) ? DioDisStatus[48] : DioPortReg[48]) ; 

//////////////////////////////////////////////////////////////////////////////
//////// Socket D Pin Number //////////////
assign DioPort[145] = (GangErrSckMask[3]||(!DioPortRW[1]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[1] : DioPortReg[1]) ;    
assign DioPort[146] = (GangErrSckMask[3]||(!DioPortRW[2]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[2] : DioPortReg[2]) ;    
assign DioPort[147] = (GangErrSckMask[3]||(!DioPortRW[3]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[3] : DioPortReg[3]) ;    
assign DioPort[148] = (GangErrSckMask[3]||(!DioPortRW[4]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[4] : DioPortReg[4]) ;    
assign DioPort[149] = (GangErrSckMask[3]||(!DioPortRW[5]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[5] : DioPortReg[5]) ;    
assign DioPort[150] = (GangErrSckMask[3]||(!DioPortRW[6]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[6] : DioPortReg[6]) ;    
assign DioPort[151] = (GangErrSckMask[3]||(!DioPortRW[7]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[7] : DioPortReg[7]) ;    
assign DioPort[152] = (GangErrSckMask[3]||(!DioPortRW[8]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[8] : DioPortReg[8]) ;    
                                                                                                                                
assign DioPort[153] = (GangErrSckMask[3]||(!DioPortRW[9])) ? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[9]  : DioPortReg[9]) ;  
assign DioPort[154] = (GangErrSckMask[3]||(!DioPortRW[10]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[10] : DioPortReg[10]) ; 
assign DioPort[155] = (GangErrSckMask[3]||(!DioPortRW[11]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[11] : DioPortReg[11]) ; 
assign DioPort[156] = (GangErrSckMask[3]||(!DioPortRW[12]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[12] : DioPortReg[12]) ; 
assign DioPort[157] = (GangErrSckMask[3]||(!DioPortRW[13]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[13] : DioPortReg[13]) ; 
assign DioPort[158] = (GangErrSckMask[3]||(!DioPortRW[14]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[14] : DioPortReg[14]) ; 
assign DioPort[159] = (GangErrSckMask[3]||(!DioPortRW[15]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[15] : DioPortReg[15]) ; 
assign DioPort[160] = (GangErrSckMask[3]||(!DioPortRW[16]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[16] : DioPortReg[16]) ; 
                                                                                                                                
assign DioPort[161] = (GangErrSckMask[3]||(!DioPortRW[17]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[17] : DioPortReg[17]) ; 
assign DioPort[162] = (GangErrSckMask[3]||(!DioPortRW[18]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[18] : DioPortReg[18]) ; 
assign DioPort[163] = (GangErrSckMask[3]||(!DioPortRW[19]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[19] : DioPortReg[19]) ; 
assign DioPort[164] = (GangErrSckMask[3]||(!DioPortRW[20]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[20] : DioPortReg[20]) ; 
assign DioPort[165] = (GangErrSckMask[3]||(!DioPortRW[21]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[21] : DioPortReg[21]) ; 
assign DioPort[166] = (GangErrSckMask[3]||(!DioPortRW[22]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[22] : DioPortReg[22]) ; 
assign DioPort[167] = (GangErrSckMask[3]||(!DioPortRW[23]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[23] : DioPortReg[23]) ; 
assign DioPort[168] = (GangErrSckMask[3]||(!DioPortRW[24]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[24] : DioPortReg[24]) ; 
                                                                                                                                
assign DioPort[169] = (GangErrSckMask[3]||(!DioPortRW[25]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[25] : DioPortReg[25]) ; 
assign DioPort[170] = (GangErrSckMask[3]||(!DioPortRW[26]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[26] : DioPortReg[26]) ; 
assign DioPort[171] = (GangErrSckMask[3]||(!DioPortRW[27]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[27] : DioPortReg[27]) ; 
assign DioPort[172] = (GangErrSckMask[3]||(!DioPortRW[28]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[28] : DioPortReg[28]) ; 
assign DioPort[173] = (GangErrSckMask[3]||(!DioPortRW[29]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[29] : DioPortReg[29]) ; 
assign DioPort[174] = (GangErrSckMask[3]||(!DioPortRW[30]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[30] : DioPortReg[30]) ; 
assign DioPort[175] = (GangErrSckMask[3]||(!DioPortRW[31]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[31] : DioPortReg[31]) ; 
assign DioPort[176] = (GangErrSckMask[3]||(!DioPortRW[32]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[32] : DioPortReg[32]) ; 
                                                                                                                                
assign DioPort[177] = (GangErrSckMask[3]||(!DioPortRW[33]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[33] : DioPortReg[33]) ; 
assign DioPort[178] = (GangErrSckMask[3]||(!DioPortRW[34]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[34] : DioPortReg[34]) ; 
assign DioPort[179] = (GangErrSckMask[3]||(!DioPortRW[35]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[35] : DioPortReg[35]) ; 
assign DioPort[180] = (GangErrSckMask[3]||(!DioPortRW[36]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[36] : DioPortReg[36]) ; 
assign DioPort[181] = (GangErrSckMask[3]||(!DioPortRW[37]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[37] : DioPortReg[37]) ; 
assign DioPort[182] = (GangErrSckMask[3]||(!DioPortRW[38]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[38] : DioPortReg[38]) ; 
assign DioPort[183] = (GangErrSckMask[3]||(!DioPortRW[39]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[39] : DioPortReg[39]) ; 
assign DioPort[184] = (GangErrSckMask[3]||(!DioPortRW[40]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[40] : DioPortReg[40]) ; 
                                                                                                                                
assign DioPort[185] = (GangErrSckMask[3]||(!DioPortRW[41]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[41] : DioPortReg[41]) ; 
assign DioPort[186] = (GangErrSckMask[3]||(!DioPortRW[42]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[42] : DioPortReg[42]) ; 
assign DioPort[187] = (GangErrSckMask[3]||(!DioPortRW[43]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[43] : DioPortReg[43]) ; 
assign DioPort[188] = (GangErrSckMask[3]||(!DioPortRW[44]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[44] : DioPortReg[44]) ; 
assign DioPort[189] = (GangErrSckMask[3]||(!DioPortRW[45]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[45] : DioPortReg[45]) ; 
assign DioPort[190] = (GangErrSckMask[3]||(!DioPortRW[46]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[46] : DioPortReg[46]) ; 
assign DioPort[191] = (GangErrSckMask[3]||(!DioPortRW[47]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[47] : DioPortReg[47]) ; 
assign DioPort[192] = (GangErrSckMask[3]||(!DioPortRW[48]))? 1'bz :((!EnSelSocketNum[3]) ? DioDisStatus[48] : DioPortReg[48]) ; 


endmodule

