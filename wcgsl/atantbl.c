#include <base.h>

uint16 ArcTanTable[2049] = {
        0,     5,    10,    15,    20,    25,    30,    35, 
       40,    45,    50,    56,    61,    66,    71,    76, 
       81,    86,    91,    96,   101,   106,   112,   117, 
      122,   127,   132,   137,   142,   147,   152,   157, 
      162,   168,   173,   178,   183,   188,   193,   198, 
      203,   208,   213,   218,   224,   229,   234,   239, 
      244,   249,   254,   259,   264,   269,   274,   280, 
      285,   290,   295,   300,   305,   310,   315,   320, 
      325,   330,   336,   341,   346,   351,   356,   361, 
      366,   371,   376,   381,   386,   391,   397,   402, 
      407,   412,   417,   422,   427,   432,   437,   442, 
      447,   452,   458,   463,   468,   473,   478,   483, 
      488,   493,   498,   503,   508,   513,   519,   524, 
      529,   534,   539,   544,   549,   554,   559,   564, 
      569,   574,   579,   585,   590,   595,   600,   605, 
      610,   615,   620,   625,   630,   635,   640,   645, 
      651,   656,   661,   666,   671,   676,   681,   686, 
      691,   696,   701,   706,   711,   716,   722,   727, 
      732,   737,   742,   747,   752,   757,   762,   767, 
      772,   777,   782,   787,   792,   798,   803,   808, 
      813,   818,   823,   828,   833,   838,   843,   848, 
      853,   858,   863,   868,   873,   878,   884,   889, 
      894,   899,   904,   909,   914,   919,   924,   929, 
      934,   939,   944,   949,   954,   959,   964,   969, 
      974,   980,   985,   990,   995,  1000,  1005,  1010, 
     1015,  1020,  1025,  1030,  1035,  1040,  1045,  1050, 
     1055,  1060,  1065,  1070,  1075,  1080,  1085,  1090, 
     1096,  1101,  1106,  1111,  1116,  1121,  1126,  1131, 
     1136,  1141,  1146,  1151,  1156,  1161,  1166,  1171, 
     1176,  1181,  1186,  1191,  1196,  1201,  1206,  1211, 
     1216,  1221,  1226,  1231,  1236,  1241,  1246,  1251, 
     1256,  1261,  1266,  1271,  1277,  1282,  1287,  1292, 
     1297,  1302,  1307,  1312,  1317,  1322,  1327,  1332, 
     1337,  1342,  1347,  1352,  1357,  1362,  1367,  1372, 
     1377,  1382,  1387,  1392,  1397,  1402,  1407,  1412, 
     1417,  1422,  1427,  1432,  1437,  1442,  1447,  1452, 
     1457,  1462,  1467,  1472,  1477,  1482,  1487,  1492, 
     1497,  1502,  1507,  1512,  1517,  1522,  1527,  1532, 
     1537,  1542,  1547,  1551,  1556,  1561,  1566,  1571, 
     1576,  1581,  1586,  1591,  1596,  1601,  1606,  1611, 
     1616,  1621,  1626,  1631,  1636,  1641,  1646,  1651, 
     1656,  1661,  1666,  1671,  1676,  1681,  1686,  1691, 
     1696,  1701,  1706,  1710,  1715,  1720,  1725,  1730, 
     1735,  1740,  1745,  1750,  1755,  1760,  1765,  1770, 
     1775,  1780,  1785,  1790,  1795,  1800,  1805,  1809, 
     1814,  1819,  1824,  1829,  1834,  1839,  1844,  1849, 
     1854,  1859,  1864,  1869,  1874,  1879,  1884,  1888, 
     1893,  1898,  1903,  1908,  1913,  1918,  1923,  1928, 
     1933,  1938,  1943,  1948,  1952,  1957,  1962,  1967, 
     1972,  1977,  1982,  1987,  1992,  1997,  2002,  2006, 
     2011,  2016,  2021,  2026,  2031,  2036,  2041,  2046, 
     2051,  2055,  2060,  2065,  2070,  2075,  2080,  2085, 
     2090,  2095,  2100,  2104,  2109,  2114,  2119,  2124, 
     2129,  2134,  2139,  2143,  2148,  2153,  2158,  2163, 
     2168,  2173,  2178,  2182,  2187,  2192,  2197,  2202, 
     2207,  2212,  2217,  2221,  2226,  2231,  2236,  2241, 
     2246,  2251,  2255,  2260,  2265,  2270,  2275,  2280, 
     2285,  2289,  2294,  2299,  2304,  2309,  2314,  2319, 
     2323,  2328,  2333,  2338,  2343,  2348,  2352,  2357, 
     2362,  2367,  2372,  2377,  2381,  2386,  2391,  2396, 
     2401,  2406,  2410,  2415,  2420,  2425,  2430,  2435, 
     2439,  2444,  2449,  2454,  2459,  2463,  2468,  2473, 
     2478,  2483,  2488,  2492,  2497,  2502,  2507,  2512, 
     2516,  2521,  2526,  2531,  2536,  2540,  2545,  2550, 
     2555,  2560,  2564,  2569,  2574,  2579,  2583,  2588, 
     2593,  2598,  2603,  2607,  2612,  2617,  2622,  2626, 
     2631,  2636,  2641,  2646,  2650,  2655,  2660,  2665, 
     2669,  2674,  2679,  2684,  2688,  2693,  2698,  2703, 
     2708,  2712,  2717,  2722,  2727,  2731,  2736,  2741, 
     2746,  2750,  2755,  2760,  2765,  2769,  2774,  2779, 
     2784,  2788,  2793,  2798,  2802,  2807,  2812,  2817, 
     2821,  2826,  2831,  2836,  2840,  2845,  2850,  2854, 
     2859,  2864,  2869,  2873,  2878,  2883,  2887,  2892, 
     2897,  2902,  2906,  2911,  2916,  2920,  2925,  2930, 
     2935,  2939,  2944,  2949,  2953,  2958,  2963,  2967, 
     2972,  2977,  2981,  2986,  2991,  2996,  3000,  3005, 
     3010,  3014,  3019,  3024,  3028,  3033,  3038,  3042, 
     3047,  3052,  3056,  3061,  3066,  3070,  3075,  3080, 
     3084,  3089,  3094,  3098,  3103,  3108,  3112,  3117, 
     3122,  3126,  3131,  3135,  3140,  3145,  3149,  3154, 
     3159,  3163,  3168,  3173,  3177,  3182,  3187,  3191, 
     3196,  3200,  3205,  3210,  3214,  3219,  3224,  3228, 
     3233,  3237,  3242,  3247,  3251,  3256,  3260,  3265, 
     3270,  3274,  3279,  3284,  3288,  3293,  3297,  3302, 
     3307,  3311,  3316,  3320,  3325,  3329,  3334,  3339, 
     3343,  3348,  3352,  3357,  3362,  3366,  3371,  3375, 
     3380,  3384,  3389,  3394,  3398,  3403,  3407,  3412, 
     3416,  3421,  3426,  3430,  3435,  3439,  3444,  3448, 
     3453,  3458,  3462,  3467,  3471,  3476,  3480,  3485, 
     3489,  3494,  3498,  3503,  3508,  3512,  3517,  3521, 
     3526,  3530,  3535,  3539,  3544,  3548,  3553,  3557, 
     3562,  3566,  3571,  3575,  3580,  3584,  3589,  3593, 
     3598,  3603,  3607,  3612,  3616,  3621,  3625,  3630, 
     3634,  3639,  3643,  3648,  3652,  3657,  3661,  3666, 
     3670,  3674,  3679,  3683,  3688,  3692,  3697,  3701, 
     3706,  3710,  3715,  3719,  3724,  3728,  3733,  3737, 
     3742,  3746,  3751,  3755,  3759,  3764,  3768,  3773, 
     3777,  3782,  3786,  3791,  3795,  3800,  3804,  3808, 
     3813,  3817,  3822,  3826,  3831,  3835,  3839,  3844, 
     3848,  3853,  3857,  3862,  3866,  3870,  3875,  3879, 
     3884,  3888,  3893,  3897,  3901,  3906,  3910,  3915, 
     3919,  3923,  3928,  3932,  3937,  3941,  3945,  3950, 
     3954,  3959,  3963,  3967,  3972,  3976,  3981,  3985, 
     3989,  3994,  3998,  4003,  4007,  4011,  4016,  4020, 
     4024,  4029,  4033,  4038,  4042,  4046,  4051,  4055, 
     4059,  4064,  4068,  4072,  4077,  4081,  4085,  4090, 
     4094,  4099,  4103,  4107,  4112,  4116,  4120,  4125, 
     4129,  4133,  4138,  4142,  4146,  4151,  4155,  4159, 
     4164,  4168,  4172,  4176,  4181,  4185,  4189,  4194, 
     4198,  4202,  4207,  4211,  4215,  4220,  4224,  4228, 
     4233,  4237,  4241,  4245,  4250,  4254,  4258,  4263, 
     4267,  4271,  4275,  4280,  4284,  4288,  4293,  4297, 
     4301,  4305,  4310,  4314,  4318,  4322,  4327,  4331, 
     4335,  4340,  4344,  4348,  4352,  4357,  4361,  4365, 
     4369,  4374,  4378,  4382,  4386,  4391,  4395,  4399, 
     4403,  4407,  4412,  4416,  4420,  4424,  4429,  4433, 
     4437,  4441,  4446,  4450,  4454,  4458,  4462,  4467, 
     4471,  4475,  4479,  4483,  4488,  4492,  4496,  4500, 
     4505,  4509,  4513,  4517,  4521,  4525,  4530,  4534, 
     4538,  4542,  4546,  4551,  4555,  4559,  4563,  4567, 
     4572,  4576,  4580,  4584,  4588,  4592,  4597,  4601, 
     4605,  4609,  4613,  4617,  4622,  4626,  4630,  4634, 
     4638,  4642,  4646,  4651,  4655,  4659,  4663,  4667, 
     4671,  4675,  4680,  4684,  4688,  4692,  4696,  4700, 
     4704,  4708,  4713,  4717,  4721,  4725,  4729,  4733, 
     4737,  4741,  4746,  4750,  4754,  4758,  4762,  4766, 
     4770,  4774,  4778,  4782,  4787,  4791,  4795,  4799, 
     4803,  4807,  4811,  4815,  4819,  4823,  4827,  4831, 
     4836,  4840,  4844,  4848,  4852,  4856,  4860,  4864, 
     4868,  4872,  4876,  4880,  4884,  4888,  4892,  4896, 
     4901,  4905,  4909,  4913,  4917,  4921,  4925,  4929, 
     4933,  4937,  4941,  4945,  4949,  4953,  4957,  4961, 
     4965,  4969,  4973,  4977,  4981,  4985,  4989,  4993, 
     4997,  5001,  5005,  5009,  5013,  5017,  5021,  5025, 
     5029,  5033,  5037,  5041,  5045,  5049,  5053,  5057, 
     5061,  5065,  5069,  5073,  5077,  5081,  5085,  5089, 
     5093,  5097,  5101,  5105,  5109,  5113,  5117,  5121, 
     5125,  5129,  5133,  5137,  5141,  5145,  5148,  5152, 
     5156,  5160,  5164,  5168,  5172,  5176,  5180,  5184, 
     5188,  5192,  5196,  5200,  5204,  5208,  5211,  5215, 
     5219,  5223,  5227,  5231,  5235,  5239,  5243,  5247, 
     5251,  5255,  5258,  5262,  5266,  5270,  5274,  5278, 
     5282,  5286,  5290,  5293,  5297,  5301,  5305,  5309, 
     5313,  5317,  5321,  5325,  5328,  5332,  5336,  5340, 
     5344,  5348,  5352,  5356,  5359,  5363,  5367,  5371, 
     5375,  5379,  5383,  5386,  5390,  5394,  5398,  5402, 
     5406,  5409,  5413,  5417,  5421,  5425,  5429,  5432, 
     5436,  5440,  5444,  5448,  5452,  5455,  5459,  5463, 
     5467,  5471,  5475,  5478,  5482,  5486,  5490,  5494, 
     5497,  5501,  5505,  5509,  5513,  5516,  5520,  5524, 
     5528,  5532,  5535,  5539,  5543,  5547,  5550,  5554, 
     5558,  5562,  5566,  5569,  5573,  5577,  5581,  5584, 
     5588,  5592,  5596,  5600,  5603,  5607,  5611,  5615, 
     5618,  5622,  5626,  5630,  5633,  5637,  5641,  5645, 
     5648,  5652,  5656,  5659,  5663,  5667,  5671,  5674, 
     5678,  5682,  5686,  5689,  5693,  5697,  5700,  5704, 
     5708,  5712,  5715,  5719,  5723,  5726,  5730,  5734, 
     5738,  5741,  5745,  5749,  5752,  5756,  5760,  5763, 
     5767,  5771,  5774,  5778,  5782,  5786,  5789,  5793, 
     5797,  5800,  5804,  5808,  5811,  5815,  5819,  5822, 
     5826,  5830,  5833,  5837,  5841,  5844,  5848,  5852, 
     5855,  5859,  5862,  5866,  5870,  5873,  5877,  5881, 
     5884,  5888,  5892,  5895,  5899,  5902,  5906,  5910, 
     5913,  5917,  5921,  5924,  5928,  5931,  5935,  5939, 
     5942,  5946,  5949,  5953,  5957,  5960,  5964,  5968, 
     5971,  5975,  5978,  5982,  5985,  5989,  5993,  5996, 
     6000,  6003,  6007,  6011,  6014,  6018,  6021,  6025, 
     6028,  6032,  6036,  6039,  6043,  6046,  6050,  6053, 
     6057,  6061,  6064,  6068,  6071,  6075,  6078,  6082, 
     6085,  6089,  6093,  6096,  6100,  6103,  6107,  6110, 
     6114,  6117,  6121,  6124,  6128,  6131,  6135,  6138, 
     6142,  6146,  6149,  6153,  6156,  6160,  6163,  6167, 
     6170,  6174,  6177,  6181,  6184,  6188,  6191,  6195, 
     6198,  6202,  6205,  6209,  6212,  6216,  6219,  6223, 
     6226,  6230,  6233,  6236,  6240,  6243,  6247,  6250, 
     6254,  6257,  6261,  6264,  6268,  6271,  6275,  6278, 
     6282,  6285,  6288,  6292,  6295,  6299,  6302,  6306, 
     6309,  6313,  6316,  6320,  6323,  6326,  6330,  6333, 
     6337,  6340,  6344,  6347,  6350,  6354,  6357,  6361, 
     6364,  6368,  6371,  6374,  6378,  6381,  6385,  6388, 
     6391,  6395,  6398,  6402,  6405,  6408,  6412,  6415, 
     6419,  6422,  6425,  6429,  6432,  6436,  6439,  6442, 
     6446,  6449,  6453,  6456,  6459,  6463,  6466,  6469, 
     6473,  6476,  6480,  6483,  6486,  6490,  6493,  6496, 
     6500,  6503,  6506,  6510,  6513,  6516,  6520,  6523, 
     6527,  6530,  6533,  6537,  6540,  6543,  6547,  6550, 
     6553,  6557,  6560,  6563,  6567,  6570,  6573,  6577, 
     6580,  6583,  6586,  6590,  6593,  6596,  6600,  6603, 
     6606,  6610,  6613,  6616,  6620,  6623,  6626,  6629, 
     6633,  6636,  6639,  6643,  6646,  6649,  6653,  6656, 
     6659,  6662,  6666,  6669,  6672,  6676,  6679,  6682, 
     6685,  6689,  6692,  6695,  6698,  6702,  6705,  6708, 
     6711,  6715,  6718,  6721,  6724,  6728,  6731,  6734, 
     6737,  6741,  6744,  6747,  6750,  6754,  6757,  6760, 
     6763,  6767,  6770,  6773,  6776,  6780,  6783,  6786, 
     6789,  6792,  6796,  6799,  6802,  6805,  6809,  6812, 
     6815,  6818,  6821,  6825,  6828,  6831,  6834,  6837, 
     6841,  6844,  6847,  6850,  6853,  6857,  6860,  6863, 
     6866,  6869,  6873,  6876,  6879,  6882,  6885,  6888, 
     6892,  6895,  6898,  6901,  6904,  6907,  6911,  6914, 
     6917,  6920,  6923,  6926,  6930,  6933,  6936,  6939, 
     6942,  6945,  6949,  6952,  6955,  6958,  6961,  6964, 
     6967,  6971,  6974,  6977,  6980,  6983,  6986,  6989, 
     6992,  6996,  6999,  7002,  7005,  7008,  7011,  7014, 
     7017,  7021,  7024,  7027,  7030,  7033,  7036,  7039, 
     7042,  7045,  7048,  7052,  7055,  7058,  7061,  7064, 
     7067,  7070,  7073,  7076,  7079,  7082,  7086,  7089, 
     7092,  7095,  7098,  7101,  7104,  7107,  7110,  7113, 
     7116,  7119,  7122,  7126,  7129,  7132,  7135,  7138, 
     7141,  7144,  7147,  7150,  7153,  7156,  7159,  7162, 
     7165,  7168,  7171,  7174,  7177,  7180,  7183,  7186, 
     7190,  7193,  7196,  7199,  7202,  7205,  7208,  7211, 
     7214,  7217,  7220,  7223,  7226,  7229,  7232,  7235, 
     7238,  7241,  7244,  7247,  7250,  7253,  7256,  7259, 
     7262,  7265,  7268,  7271,  7274,  7277,  7280,  7283, 
     7286,  7289,  7292,  7295,  7298,  7301,  7304,  7307, 
     7310,  7313,  7316,  7319,  7322,  7325,  7328,  7330, 
     7333,  7336,  7339,  7342,  7345,  7348,  7351,  7354, 
     7357,  7360,  7363,  7366,  7369,  7372,  7375,  7378, 
     7381,  7384,  7387,  7389,  7392,  7395,  7398,  7401, 
     7404,  7407,  7410,  7413,  7416,  7419,  7422,  7425, 
     7428,  7430,  7433,  7436,  7439,  7442,  7445,  7448, 
     7451,  7454,  7457,  7460,  7462,  7465,  7468,  7471, 
     7474,  7477,  7480,  7483,  7486,  7489,  7491,  7494, 
     7497,  7500,  7503,  7506,  7509,  7512,  7514,  7517, 
     7520,  7523,  7526,  7529,  7532,  7535,  7537,  7540, 
     7543,  7546,  7549,  7552,  7555,  7557,  7560,  7563, 
     7566,  7569,  7572,  7575,  7577,  7580,  7583,  7586, 
     7589,  7592,  7594,  7597,  7600,  7603,  7606,  7609, 
     7611,  7614,  7617,  7620,  7623,  7626,  7628,  7631, 
     7634,  7637,  7640,  7642,  7645,  7648,  7651,  7654, 
     7657,  7659,  7662,  7665,  7668,  7671,  7673,  7676, 
     7679,  7682,  7685,  7687,  7690,  7693,  7696,  7698, 
     7701,  7704,  7707,  7710,  7712,  7715,  7718,  7721, 
     7724,  7726,  7729,  7732,  7735,  7737,  7740,  7743, 
     7746,  7748,  7751,  7754,  7757,  7759,  7762,  7765, 
     7768,  7770,  7773,  7776,  7779,  7781,  7784,  7787, 
     7790,  7792,  7795,  7798,  7801,  7803,  7806,  7809, 
     7812,  7814,  7817,  7820,  7823,  7825,  7828,  7831, 
     7833,  7836,  7839,  7842,  7844,  7847,  7850,  7852, 
     7855,  7858,  7861,  7863,  7866,  7869,  7871,  7874, 
     7877,  7879,  7882,  7885,  7888,  7890,  7893,  7896, 
     7898,  7901,  7904,  7906,  7909,  7912,  7914,  7917, 
     7920,  7923,  7925,  7928,  7931,  7933,  7936,  7939, 
     7941,  7944,  7947,  7949,  7952,  7955,  7957,  7960, 
     7963,  7965,  7968,  7970,  7973,  7976,  7978,  7981, 
     7984,  7986,  7989,  7992,  7994,  7997,  8000,  8002, 
     8005,  8008,  8010,  8013,  8015,  8018,  8021,  8023, 
     8026,  8029,  8031,  8034,  8036,  8039,  8042,  8044, 
     8047,  8050,  8052,  8055,  8057,  8060,  8063,  8065, 
     8068,  8070,  8073,  8076,  8078,  8081,  8083,  8086, 
     8089,  8091,  8094,  8096,  8099,  8102,  8104,  8107, 
     8109,  8112,  8115,  8117,  8120,  8122,  8125,  8127, 
     8130,  8133,  8135,  8138,  8140,  8143,  8145,  8148, 
     8151,  8153,  8156,  8158,  8161,  8163,  8166,  8169, 
     8171,  8174,  8176,  8179,  8181,  8184,  8186,  8189, 
     8192
};
