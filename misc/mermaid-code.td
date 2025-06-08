graph TD
    %% 取指阶段
    PC[程序计数器<br/>PC] --> IM[指令存储器<br/>Memory]
    PC --> ADD1[+4]
    ADD1 --> PC
    IM --> IR[指令寄存器<br/>IR]
    
    %% 译码阶段
    IR --> DEC[译码器<br/>decode]
    DEC --> CTRL[控制信号]
    
    %% 寄存器文件
    DEC --> RF[寄存器文件<br/>32个64位寄存器]
    RF --> |rn| ALU_A[操作数A]
    RF --> |rm| ALU_B1[操作数B1]
    
    %% 立即数扩展
    DEC --> IMM[立即数<br/>符号扩展]
    IMM --> MUX1{MUX}
    ALU_B1 --> MUX1
    MUX1 --> |选择操作数B| ALU_B[操作数B]
    
    %% ALU单元
    ALU_A --> ALU[ALU<br/>算术逻辑单元]
    ALU_B --> ALU
    CTRL --> |ALU控制| ALU
    ALU --> |结果| ALU_OUT[ALU输出]
    ALU --> |标志位| STATUS[状态寄存器<br/>NZCV]
    
    %% 数据存储器
    ALU_OUT --> DM[数据存储器<br/>Memory]
    DM --> |加载数据| MUX2{MUX}
    ALU_OUT --> MUX2
    
    %% 写回多路选择器
    MUX2 --> |写回数据| RF
    
    %% 分支控制
    STATUS --> BC[分支条件<br/>检查]
    BC --> |分支控制| MUX3{MUX}
    ADD1 --> MUX3
    ALU_OUT --> |分支目标| MUX3
    MUX3 --> PC
    
    %% 系统寄存器
    RF --> |X30 LR| LR[链接寄存器]
    RF --> |X31 SP| SP[栈指针]
    
    %% 控制信号标注
    CTRL --> |reg_write| RF
    CTRL --> |mem_read| DM
    CTRL --> |mem_write| DM
    CTRL --> |alu_src| MUX1
    CTRL --> |reg_dst| MUX2
    CTRL --> |branch| BC
    CTRL --> |pc_src| MUX3
    
    %% 样式设置
    classDef memory fill:#e1f5fe
    classDef register fill:#f3e5f5
    classDef alu fill:#fff3e0
    classDef control fill:#ffebee
    classDef mux fill:#e8f5e8
    
    class IM,DM memory
    class PC,IR,RF,STATUS,LR,SP register
    class ALU alu
    class DEC,CTRL,BC control
    class MUX1,MUX2,MUX3 mux