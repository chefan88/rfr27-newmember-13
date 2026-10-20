long vref_mv = 5000;
long ADC = 1024L;
long AMP = 2L;
long Open_Sens = 2470;
long Shorted_Sens = 1200;
int Open = 1;
int Shorted = 2;
int Normal = 0;
const int table_length = 33;
int Step = 5;
int Temp_Min = -40;

static const int temp_table[table_length] = {
  2440, 2420, 2400, 2380, 2350, 2320, 2270, 2230, 2170, 2110, 2050, 1990,
  1920, 1860, 1800, 1740, 1680, 1630, 1590, 1550, 1510, 1480, 1450, 1430,
  1400, 1380, 1370, 1350, 1340, 1330, 1320, 1310, 1300
};

static long count_calc(int count){
    long val1 = (long)count * vref_mv;
    long val2 = ADC * AMP;
    return (val1 + val2 / 2) / val2;
}

