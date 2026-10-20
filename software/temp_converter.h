#define vref_mv = 5000L;
#define ADC = 1024L;
#define AMP = 2L;
#define Open_Sens = 2470;
#define Shorted_Sens = 1200;
#define Open = 1;
#define Shorted = 2;
#define Normal = 0;
const int table_length = 33;
#define Step = 5;
#define Temp_Min = -40;

static const int temp_table[table_length] = {
  2440, 2420, 2400, 2380, 2350, 2320, 2270, 2230, 2170, 2110, 2050, 1990,
  1920, 1860, 1800, 1740, 1680, 1630, 1590, 1550, 1510, 1480, 1450, 1430,
  1400, 1380, 1370, 1350, 1340, 1330, 1320, 1310, 1300
};

