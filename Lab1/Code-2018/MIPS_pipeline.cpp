#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab,
//for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.
#define DEBUG true

struct IFStruct {
    bitset<32>  PC = 0;
    bool        nop = true;
};

struct IDStruct {
    bitset<32>  Instr = bitset<32>(string(32, '1'));
    bool        nop = true;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type = false;
    bool        rd_mem = false;
    bool        wrt_mem = false;
    bool        alu_op = true;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable = false;
    bool        nop = true;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem = false;
    bool        wrt_mem = false;
    bool        wrt_enable = false;
    bool        nop = true;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable = false;
    bool        nop = true;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}

bitset<32> signExt(bitset<16> Imm){

    /* Sign Extension
     * */

    string Imm_str = Imm.to_string();
    char sign = Imm_str.at(0);
    Imm_str = string(16, sign) + Imm_str;
    return bitset<32>(Imm_str);

}

bitset<32> branAddr(bitset<16> Imm){

    /* Sign Extension
     * */

    string Imm_str = Imm.to_string();
    char sign = Imm_str.at(0);
    Imm_str = string(14, sign) + Imm_str + string(2, '0');
    return bitset<32>(Imm_str);

}

int main()
{
    
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;

    /* Initializing state
     * Set all state nop except for IF stage
     * Set initial address to 0
     * */

    stateStruct state;

    state.IF.nop = false;
    state.IF.PC = bitset<32> (0);

    int cycle = 0;

    while (1) {

        stateStruct newState;

        if (DEBUG) cout<<"cycle :"<<cycle<<" begins"<<endl;

        /* --------------------- WB stage --------------------- */

        if (! state.WB.nop and state.WB.wrt_enable){
            /* Do write back stage if enable */
            myRF.writeRF (state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            if (DEBUG) cout<<" write back " <<state.WB.Wrt_data.to_string()<< " to R["<<state.WB.Wrt_reg_addr.to_ulong()<<"]"<<endl;
            /* No forward checking here since nothing to Fw
             * */
        }

        /* --------------------- MEM stage --------------------- */

        if (state.MEM.nop) {
            newState.WB.nop = true;
        } else {
            newState.WB.nop = false;
            /* Do MEM stage
             * */

            /* TO DO: Forward Checking
             * Only need to concern for sw ?
             * */
            if (!state.WB.nop and state.WB.wrt_enable) {
                if (state.MEM.wrt_mem and state.MEM.Rt == state.WB.Wrt_reg_addr){
                    if (DEBUG) cout<<"  MEM -> MEN forwarding "<<state.WB.Wrt_data.to_string()<<" to R["<<state.MEM.Rt.to_ulong()<<"]"<<endl;
                    state.MEM.Store_data = state.WB.Wrt_data;
                }
            }

            /* Hey infack read and write can only happen once per time, or totally no
             * */

            if (state.MEM.wrt_mem){
                if (DEBUG) cout<<" mem write to M["<<state.MEM.ALUresult.to_ulong()<<"] with value: "<<state.MEM.Store_data.to_string()<<endl;
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
            }

            if (state.MEM.rd_mem){
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            } else {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }

            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;

        }

        /* --------------------- EX stage --------------------- */

        if (state.EX.nop){
            newState.MEM.nop = true;
        } else {
            newState.MEM.nop = false;

            /* TO DO: Forward Checking
             * Only when nop and wrt_enable
             * In Ex -> Ex stage, lw as parent can be problematic and do a stall !
             * Do not need to check in WB stage
             * */

            if (!state.WB.nop and state.WB.wrt_enable){

                /* Check the first register !
                 * */

                if (state.WB.Wrt_reg_addr == state.EX.Rs){
                    /* Do the forwarding to Op 1
                     * */
                    if (DEBUG) cout<<"  MEM -> Ex stage forwarding to Rs: "<<state.EX.Rs.to_string()<<endl;
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }

                if (state.WB.Wrt_reg_addr == state.EX.Rt){
                    /* Check the second register !
                     * */
                    if (DEBUG) cout<<"  MEM -> Ex stage forwarding to Rt: "<<state.EX.Rt.to_string()<<endl;
                    state.EX.Read_data2 = state.WB.Wrt_data;
                }


            }

            if (!state.MEM.nop and !state.MEM.rd_mem and state.MEM.wrt_enable){
                /* Ex -> Ex forwarding have priprity
                 * if parent is load word (rd_mem = true) in Ex -> Ex, stall, do not forward!
                 * check if nop
                 * */

                if (state.MEM.Wrt_reg_addr == state.EX.Rs){
                    /* Do the forwarding to Op 1
                     * */
                    state.EX.Read_data1 = state.MEM.ALUresult;
                    if (DEBUG) cout<<"  Ex -> Ex stage forwarding to Rs: "<<state.EX.Rs.to_string()<<endl;

                }

                if (!state.EX.is_I_type and state.MEM.Wrt_reg_addr == state.EX.Rt){
                    /* not an I type instruction
                     * */
                    state.EX.Read_data2 = state.MEM.ALUresult;
                    if (DEBUG) cout<<"  Ex -> Ex stage forwarding to Rt: "<<state.EX.Rt.to_string()<<endl;

                }
            }

            /* ALU operation, should happen at the end
             * Itpye = Rs + Imm
             * Add u = Rs + Rt
             * Sub u = Rs - Rt
             * */

            if (state.EX.is_I_type){
                newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong()
                        + (int32_t) signExt(state.EX.Imm).to_ulong());
            } else {
                if (state.EX.alu_op){
                    newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong()
                            + state.EX.Read_data2.to_ulong());
                } else {
                    newState.MEM.ALUresult = bitset<32> (state.EX.Read_data1.to_ulong()
                            - state.EX.Read_data2.to_ulong());
                }
            }

            newState.MEM.Store_data = state.EX.Read_data2;

            /*
            if (state.EX.wrt_mem) {
                newState.MEM.Store_data = state.EX.Read_data2;
            }
             */

            /* Passing to mem state
             * */

            newState.MEM.Rs = state.EX.Rs;
            // which register that rd1 came from
            newState.MEM.Rt = state.EX.Rt;
            // which register that rd2 came from
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            // which register to write to
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;

        }

        /* --------------------- ID/RF stage --------------------- */

        bool stall = false;
        bool jump = false;
        bool isbeq = false;

        bitset<32> branchAddr = bitset<32> (0);

        if (state.ID.nop){
            newState.EX.nop = true;
        } else {

            /* Addu: 000000 + 100001 R[rd] = R[rs] + R[rt]
             * Subu: 000000 + 100011 R R[rd] = R[rs] - R[rt]
             * Lw: 100011 R[rt] = M[R[rs]+SignExtImm]
             * Sw: 101011 M[R[rs]+SignExtImm] = R[rt]
             * Beq: 000100
             * */

            string Inst_31_26 = state.ID.Instr.to_string().substr(0,6);
            string Inst_5_0 = state.ID.Instr.to_string().substr(26,6);

            bitset<5> rs = bitset<5> (state.ID.Instr.to_string().substr(6,5));
            bitset<5> rt = bitset<5> (state.ID.Instr.to_string().substr(11,5));
            bitset<5> rd = bitset<5> (state.ID.Instr.to_string().substr(16,5));
            bitset<16> imm = bitset<16> (state.ID.Instr.to_string().substr(16,16));

            newState.EX.Read_data1 = myRF.readRF(rs);
            newState.EX.Read_data2 = myRF.readRF(rt);
            newState.EX.Imm = imm;

            newState.EX.is_I_type = (Inst_31_26 != "000000") and (Inst_31_26 != "000010");

            if (newState.EX.is_I_type){
                newState.EX.Wrt_reg_addr = rt;
            } else {
                newState.EX.Wrt_reg_addr = rd;
            }

            newState.EX.alu_op = (Inst_31_26 == "000000" and Inst_5_0 == "100001") or newState.EX.is_I_type;
            newState.EX.wrt_enable = (Inst_31_26 != "101011") and (Inst_31_26 != "000100");
            newState.EX.Rs = rs;
            newState.EX.Rt = rt;
            newState.EX.rd_mem = Inst_31_26 == "100011";
            newState.EX.wrt_mem = Inst_31_26 == "101011";
            newState.EX.nop = false;

            /* Stall condition, check the out of EX, if it
             * */

            if (!state.EX.nop and state.EX.rd_mem){

                /* This happens only when a load word instruction is parent !
                 * when followed by addu/subu, stall if rs/rt = rt
                 * when followed by sw, stall only if rs = rt
                 * */

                if (!newState.EX.is_I_type and (state.EX.Rt == rs or state.EX.Rt == rt)) stall = true;
                if (newState.EX.is_I_type and state.EX.Rt == rs) stall = true;

                if (stall and DEBUG){
                    cout<<"  stall type"<<newState.EX.is_I_type;
                    cout<<"  stall since Ex rt ["<<state.EX.Rt.to_ulong()<<"] and rs ["<<rs.to_ulong()<<"] rt["<<rt.to_ulong()<<"]"<<endl;
                }
            }

            if (Inst_31_26 == "000100"){
                /* bne instruction!
                 * Jump if not equal
                 * */
                newState.EX.nop = true;
                if (newState.EX.Read_data1 != newState.EX.Read_data2){
                    jump = true;
                    branchAddr = branAddr(newState.EX.Imm);
                    if (DEBUG) cout<<"jump to extra: "<< (int32_t) branchAddr.to_ulong() <<endl;
                }

            }

            if (DEBUG) {

                if (Inst_31_26 == "000000"){
                    if (Inst_5_0 == "100001"){
                        cout<<" Decode addu R["<<rs.to_ulong()<<"] and R["<<rt.to_ulong()<<"] to R["<<rd.to_ulong()<<"]"<<endl;
                    } else {
                        cout<<" Decode subu R["<<rs.to_ulong()<<"] and R["<<rt.to_ulong()<<"] to R["<<rd.to_ulong()<<"]"<<endl;
                    }

                } else {
                    if (Inst_31_26 == "100011"){
                        cout<<" Decode load MEM[R["<<rs.to_ulong()<<"] + SignExt "<<imm.to_string()<<"] to R["<<rt.to_ulong()<<"]"<<endl;
                    }

                    if (Inst_31_26 == "101011"){
                        cout<<" Decode write R["<<rt.to_ulong()<<"] to MEM[R["<<rs.to_ulong()<<"] + SignExt "<<imm.to_string()<<"]"<<endl;
                    }

                    if (Inst_31_26 == "101011"){
                        cout<<" Decode bne R["<<rt.to_ulong()<<"] != R["<<rs.to_ulong()<<"] to Branch Ext "<<imm.to_string()<<endl;
                    }

                }
            }
        }

        /* --------------------- IF stage --------------------- */

        if (state.IF.nop){
            /* this happens only after halt ... */
            newState.IF.nop = true;
            newState.ID.nop = true;

        } else {
            /* Do IF stage
             * Read memory form current state file
             * */
            myInsMem.readInstr(state.IF.PC);
            newState.ID.Instr = myInsMem.Instruction;

            if (DEBUG) {cout<<" Fetch "<<myInsMem.Instruction.to_string()<<endl;}

            if (myInsMem.Instruction.all()){
                /* Halt
                 * nop decode next cycle
                 * nop fetch next cycle
                 * */
                newState.ID.nop = true;
                newState.IF.nop = true;

            } else {
                /* Not a halt, pass instruction to decoder
                 * PC += 4
                 * */
                newState.ID.nop = false;
                newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
                newState.IF.nop = false;
            }

        }

        if (stall){
            /* stall the if and id/rf
             * */
            newState.IF = state.IF;
            newState.ID = state.ID;

            /* push a nop to next stage
             * */
            newState.EX.nop = true;
        }

        if (jump){
            newState.ID.nop = true;
            newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + (int32_t) branchAddr.to_ulong());
        }

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
             break;
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...
       
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

        cycle += 1;

    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}