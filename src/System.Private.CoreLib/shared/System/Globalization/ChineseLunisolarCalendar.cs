// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

namespace System.Globalization
{
    /// <remarks>
    /// Calendar support range:
    ///     Calendar               Minimum             Maximum
    ///     ==========             ==========          ==========
    ///     Gregorian              1901/02/19          2101/01/28
    ///     ChineseLunisolar       1901/01/01          2100/12/29
    /// </remarks>

    public class ChineseLunisolarCalendar : EastAsianLunisolarCalendar
    {
        public const int ChineseEra = 1;

        private const int MinLunisolarYear = 1901;
        private const int MaxLunisolarYear = 2100;

        private static readonly DateTime s_minDate = new DateTime(1901, 2, 19);
        private static readonly DateTime s_maxDate = new DateTime((new DateTime(2101, 1, 28, 23, 59, 59, 999)).Ticks + 9999);

        public override DateTime MinSupportedDateTime => s_minDate;

        public override DateTime MaxSupportedDateTime => s_maxDate;

        protected override int DaysInYearBeforeMinSupportedYear
        {
            get
            {
                // 1900: 1-29 2-30 3-29 4-29 5-30 6-29 7-30 8-30 Leap8-29 9-30 10-30 11-29 12-30 from Calendrical Tabulations [1]
                // [1] Reingold, Edward M, and Nachum Dershowitz. Calendrical Tabulations, 1900 - 2200.Cambridge: Cambridge Univ. Press, 2002.Print.
                return 384;
            }
        }

        // Data for years 1901-1905 and 1907-2100 matches output of Calendrical Calculations [2] and published calendar tables [3].
        // For 1906, month 4 of the Chinese year starts on 24 Apr 1906 and has 29 days.  This is historially accurate
        // but different to the values in [1] and output from [2].  This is due to a change in the astronomical methods used
        // by the Chinese to calculate the calendar from 1913 onwards (see warnings in [1]).
        // [2] Reingold, Edward M, and Nachum Dershowitz. Calendrical Calculations: The Ultimate Edition. Cambridge [etc.: Cambridge University Press, 2018. Print.
        // [3] Wang, Jianmin. Xin Bian Wan Nian Li: (1840-2050) Chong Bian Ben. Beijing: Ke xue pu ji chu ban she, 1990. Print.
        private static readonly int[,] s_yinfo =
        {
/*Y           LM  Lmon  Lday    DaysPerMonth               D1   D2   D3   D4   D5   D6   D7   D8   D9   D10  D11  D12  D13  #Days
1901     */ { 00,   02,   19,   0b0100101011100000 }, /*   29   30   29   29   30   29   30   29   30   30   30   29        354
1902     */ { 00,   02,   08,   0b1010010101110000 }, /*   30   29   30   29   29   30   29   30   29   30   30   30        355
1903     */ { 05,   01,   29,   0b0101001001101000 }, /*   29   30   29   30   29   29   30   29   29   30   30   29   30   383
1904     */ { 00,   02,   16,   0b1101001001100000 }, /*   30   30   29   30   29   29   30   29   29   30   30   29        354
1905     */ { 00,   02,   04,   0b1101100101010000 }, /*   30   30   29   30   30   29   29   30   29   30   29   30        355
1906     */ { 04,   01,   25,   0b0110101010101000 }, /*   29   30   30   29   30   29   30   29   30   29   30   29   30   384
1907     */ { 00,   02,   13,   0b0101011010100000 }, /*   29   30   29   30   29   30   30   29   30   29   30   29        354
1908     */ { 00,   02,   02,   0b1001101011010000 }, /*   30   29   29   30   30   29   30   29   30   30   29   30        355
1909     */ { 02,   01,   22,   0b0100101011101000 }, /*   29   30   29   29   30   29   30   29   30   30   30   29   30   384
1910     */ { 00,   02,   10,   0b0100101011100000 }, /*   29   30   29   29   30   29   30   29   30   30   30   29        354
1911     */ { 06,   01,   30,   0b1010010011011000 }, /*   30   29   30   29   29   30   29   29   30   30   29   30   30   384
1912     */ { 00,   02,   18,   0b1010010011010000 }, /*   30   29   30   29   29   30   29   29   30   30   29   30        354
1913     */ { 00,   02,   06,   0b1101001001010000 }, /*   30   30   29   30   29   29   30   29   29   30   29   30        354
1914     */ { 05,   01,   26,   0b1101010100101000 }, /*   30   30   29   30   29   30   29   30   29   29   30   29   30   384
1915     */ { 00,   02,   14,   0b1011010101000000 }, /*   30   29   30   30   29   30   29   30   29   30   29   29        354
1916     */ { 00,   02,   03,   0b1101011010100000 }, /*   30   30   29   30   29   30   30   29   30   29   30   29        355
1917     */ { 02,   01,   23,   0b1001011011010000 }, /*   30   29   29   30   29   30   30   29   30   30   29   30   29   384
1918     */ { 00,   02,   11,   0b1001010110110000 }, /*   30   29   29   30   29   30   29   30   30   29   30   30        355
1919     */ { 07,   02,   01,   0b0100100110111000 }, /*   29   30   29   29   30   29   29   30   30   29   30   30   30   384
1920     */ { 00,   02,   20,   0b0100100101110000 }, /*   29   30   29   29   30   29   29   30   29   30   30   30        354
1921     */ { 00,   02,   08,   0b1010010010110000 }, /*   30   29   30   29   29   30   29   29   30   29   30   30        354
1922     */ { 05,   01,   28,   0b1011001001011000 }, /*   30   29   30   30   29   29   30   29   29   30   29   30   30   384
1923     */ { 00,   02,   16,   0b0110101001010000 }, /*   29   30   30   29   30   29   30   29   29   30   29   30        354
1924     */ { 00,   02,   05,   0b0110110101000000 }, /*   29   30   30   29   30   30   29   30   29   30   29   29        354
1925     */ { 04,   01,   24,   0b1010110110101000 }, /*   30   29   30   29   30   30   29   30   30   29   30   29   30   385
1926     */ { 00,   02,   13,   0b0010101101100000 }, /*   29   29   30   29   30   29   30   30   29   30   30   29        354
1927     */ { 00,   02,   02,   0b1001010101110000 }, /*   30   29   29   30   29   30   29   30   29   30   30   30        355
1928     */ { 02,   01,   23,   0b0100100101111000 }, /*   29   30   29   29   30   29   29   30   29   30   30   30   30   384
1929     */ { 00,   02,   10,   0b0100100101110000 }, /*   29   30   29   29   30   29   29   30   29   30   30   30        354
1930     */ { 06,   01,   30,   0b0110010010110000 }, /*   29   30   30   29   29   30   29   29   30   29   30   30   29   383
1931     */ { 00,   02,   17,   0b1101010010100000 }, /*   30   30   29   30   29   30   29   29   30   29   30   29        354
1932     */ { 00,   02,   06,   0b1110101001010000 }, /*   30   30   30   29   30   29   30   29   29   30   29   30        355
1933     */ { 05,   01,   26,   0b0110110101001000 }, /*   29   30   30   29   30   30   29   30   29   30   29   29   30   384
1934     */ { 00,   02,   14,   0b0101101011010000 }, /*   29   30   29   30   30   29   30   29   30   30   29   30        355
1935     */ { 00,   02,   04,   0b0010101101100000 }, /*   29   29   30   29   30   29   30   30   29   30   30   29        354
1936     */ { 03,   01,   24,   0b1001001101110000 }, /*   30   29   29   30   29   29   30   30   29   30   30   30   29   384
1937     */ { 00,   02,   11,   0b1001001011100000 }, /*   30   29   29   30   29   29   30   29   30   30   30   29        354
1938     */ { 07,   01,   31,   0b1100100101101000 }, /*   30   30   29   29   30   29   29   30   29   30   30   29   30   384
1939     */ { 00,   02,   19,   0b1100100101010000 }, /*   30   30   29   29   30   29   29   30   29   30   29   30        354
1940     */ { 00,   02,   08,   0b1101010010100000 }, /*   30   30   29   30   29   30   29   29   30   29   30   29        354
1941     */ { 06,   01,   27,   0b1101101001010000 }, /*   30   30   29   30   30   29   30   29   29   30   29   30   29   384
1942     */ { 00,   02,   15,   0b1011010101010000 }, /*   30   29   30   30   29   30   29   30   29   30   29   30        355
1943     */ { 00,   02,   05,   0b0101011010100000 }, /*   29   30   29   30   29   30   30   29   30   29   30   29        354
1944     */ { 04,   01,   25,   0b1010101011011000 }, /*   30   29   30   29   30   29   30   29   30   30   29   30   30   385
1945     */ { 00,   02,   13,   0b0010010111010000 }, /*   29   29   30   29   29   30   29   30   30   30   29   30        354
1946     */ { 00,   02,   02,   0b1001001011010000 }, /*   30   29   29   30   29   29   30   29   30   30   29   30        354
1947     */ { 02,   01,   22,   0b1100100101011000 }, /*   30   30   29   29   30   29   29   30   29   30   29   30   30   384
1948     */ { 00,   02,   10,   0b1010100101010000 }, /*   30   29   30   29   30   29   29   30   29   30   29   30        354
1949     */ { 07,   01,   29,   0b1011010010101000 }, /*   30   29   30   30   29   30   29   29   30   29   30   29   30   384
1950     */ { 00,   02,   17,   0b0110110010100000 }, /*   29   30   30   29   30   30   29   29   30   29   30   29        354
1951     */ { 00,   02,   06,   0b1011010101010000 }, /*   30   29   30   30   29   30   29   30   29   30   29   30        355
1952     */ { 05,   01,   27,   0b0101010110101000 }, /*   29   30   29   30   29   30   29   30   30   29   30   29   30   384
1953     */ { 00,   02,   14,   0b0100110110100000 }, /*   29   30   29   29   30   30   29   30   30   29   30   29        354
1954     */ { 00,   02,   03,   0b1010010110110000 }, /*   30   29   30   29   29   30   29   30   30   29   30   30        355
1955     */ { 03,   01,   24,   0b0101001010111000 }, /*   29   30   29   30   29   29   30   29   30   29   30   30   30   384
1956     */ { 00,   02,   12,   0b0101001010110000 }, /*   29   30   29   30   29   29   30   29   30   29   30   30        354
1957     */ { 08,   01,   31,   0b1010100101010000 }, /*   30   29   30   29   30   29   29   30   29   30   29   30   29   383
1958     */ { 00,   02,   18,   0b1110100101010000 }, /*   30   30   30   29   30   29   29   30   29   30   29   30        355
1959     */ { 00,   02,   08,   0b0110101010100000 }, /*   29   30   30   29   30   29   30   29   30   29   30   29        354
1960     */ { 06,   01,   28,   0b1010110101010000 }, /*   30   29   30   29   30   30   29   30   29   30   29   30   29   384
1961     */ { 00,   02,   15,   0b1010101101010000 }, /*   30   29   30   29   30   29   30   30   29   30   29   30        355
1962     */ { 00,   02,   05,   0b0100101101100000 }, /*   29   30   29   29   30   29   30   30   29   30   30   29        354
1963     */ { 04,   01,   25,   0b1010010101110000 }, /*   30   29   30   29   29   30   29   30   29   30   30   30   29   384
1964     */ { 00,   02,   13,   0b1010010101110000 }, /*   30   29   30   29   29   30   29   30   29   30   30   30        355
1965     */ { 00,   02,   02,   0b0101001001100000 }, /*   29   30   29   30   29   29   30   29   29   30   30   29        353
1966     */ { 03,   01,   21,   0b1110100100110000 }, /*   30   30   30   29   30   29   29   30   29   29   30   30   29   384
1967     */ { 00,   02,   09,   0b1101100101010000 }, /*   30   30   29   30   30   29   29   30   29   30   29   30        355
1968     */ { 07,   01,   30,   0b0101101010101000 }, /*   29   30   29   30   30   29   30   29   30   29   30   29   30   384
1969     */ { 00,   02,   17,   0b0101011010100000 }, /*   29   30   29   30   29   30   30   29   30   29   30   29        354
1970     */ { 00,   02,   06,   0b1001011011010000 }, /*   30   29   29   30   29   30   30   29   30   30   29   30        355
1971     */ { 05,   01,   27,   0b0100101011101000 }, /*   29   30   29   29   30   29   30   29   30   30   30   29   30   384
1972     */ { 00,   02,   15,   0b0100101011010000 }, /*   29   30   29   29   30   29   30   29   30   30   29   30        354
1973     */ { 00,   02,   03,   0b1010010011010000 }, /*   30   29   30   29   29   30   29   29   30   30   29   30        354
1974     */ { 04,   01,   23,   0b1101001001101000 }, /*   30   30   29   30   29   29   30   29   29   30   30   29   30   384
1975     */ { 00,   02,   11,   0b1101001001010000 }, /*   30   30   29   30   29   29   30   29   29   30   29   30        354
1976     */ { 08,   01,   31,   0b1101010100101000 }, /*   30   30   29   30   29   30   29   30   29   29   30   29   30   384
1977     */ { 00,   02,   18,   0b1011010101000000 }, /*   30   29   30   30   29   30   29   30   29   30   29   29        354
1978     */ { 00,   02,   07,   0b1011011010100000 }, /*   30   29   30   30   29   30   30   29   30   29   30   29        355
1979     */ { 06,   01,   28,   0b1001011011010000 }, /*   30   29   29   30   29   30   30   29   30   30   29   30   29   384
1980     */ { 00,   02,   16,   0b1001010110110000 }, /*   30   29   29   30   29   30   29   30   30   29   30   30        355
1981     */ { 00,   02,   05,   0b0100100110110000 }, /*   29   30   29   29   30   29   29   30   30   29   30   30        354
1982     */ { 04,   01,   25,   0b1010010010111000 }, /*   30   29   30   29   29   30   29   29   30   29   30   30   30   384
1983     */ { 00,   02,   13,   0b1010010010110000 }, /*   30   29   30   29   29   30   29   29   30   29   30   30        354
1984     */ { 10,   02,   02,   0b1011001001011000 }, /*   30   29   30   30   29   29   30   29   29   30   29   30   30   384
1985     */ { 00,   02,   20,   0b0110101001010000 }, /*   29   30   30   29   30   29   30   29   29   30   29   30        354
1986     */ { 00,   02,   09,   0b0110110101000000 }, /*   29   30   30   29   30   30   29   30   29   30   29   29        354
1987     */ { 06,   01,   29,   0b1010110110100000 }, /*   30   29   30   29   30   30   29   30   30   29   30   29   29   384
1988     */ { 00,   02,   17,   0b1010101101100000 }, /*   30   29   30   29   30   29   30   30   29   30   30   29        355
1989     */ { 00,   02,   06,   0b1001010101110000 }, /*   30   29   29   30   29   30   29   30   29   30   30   30        355
1990     */ { 05,   01,   27,   0b0100100101111000 }, /*   29   30   29   29   30   29   29   30   29   30   30   30   30   384
1991     */ { 00,   02,   15,   0b0100100101110000 }, /*   29   30   29   29   30   29   29   30   29   30   30   30        354
1992     */ { 00,   02,   04,   0b0110010010110000 }, /*   29   30   30   29   29   30   29   29   30   29   30   30        354
1993     */ { 03,   01,   23,   0b0110101001010000 }, /*   29   30   30   29   30   29   30   29   29   30   29   30   29   383
1994     */ { 00,   02,   10,   0b1110101001010000 }, /*   30   30   30   29   30   29   30   29   29   30   29   30        355
1995     */ { 08,   01,   31,   0b0110101100101000 }, /*   29   30   30   29   30   29   30   30   29   29   30   29   30   384
1996     */ { 00,   02,   19,   0b0101101011000000 }, /*   29   30   29   30   30   29   30   29   30   30   29   29        354
1997     */ { 00,   02,   07,   0b1010101101100000 }, /*   30   29   30   29   30   29   30   30   29   30   30   29        355
1998     */ { 05,   01,   28,   0b1001001101101000 }, /*   30   29   29   30   29   29   30   30   29   30   30   29   30   384
1999     */ { 00,   02,   16,   0b1001001011100000 }, /*   30   29   29   30   29   29   30   29   30   30   30   29        354
2000     */ { 00,   02,   05,   0b1100100101100000 }, /*   30   30   29   29   30   29   29   30   29   30   30   29        354
2001     */ { 04,   01,   24,   0b1101010010101000 }, /*   30   30   29   30   29   30   29   29   30   29   30   29   30   384
2002     */ { 00,   02,   12,   0b1101010010100000 }, /*   30   30   29   30   29   30   29   29   30   29   30   29        354
2003     */ { 00,   02,   01,   0b1101101001010000 }, /*   30   30   29   30   30   29   30   29   29   30   29   30        355
2004     */ { 02,   01,   22,   0b0101101010101000 }, /*   29   30   29   30   30   29   30   29   30   29   30   29   30   384
2005     */ { 00,   02,   09,   0b0101011010100000 }, /*   29   30   29   30   29   30   30   29   30   29   30   29        354
2006     */ { 07,   01,   29,   0b1010101011011000 }, /*   30   29   30   29   30   29   30   29   30   30   29   30   30   385
2007     */ { 00,   02,   18,   0b0010010111010000 }, /*   29   29   30   29   29   30   29   30   30   30   29   30        354
2008     */ { 00,   02,   07,   0b1001001011010000 }, /*   30   29   29   30   29   29   30   29   30   30   29   30        354
2009     */ { 05,   01,   26,   0b1100100101011000 }, /*   30   30   29   29   30   29   29   30   29   30   29   30   30   384
2010     */ { 00,   02,   14,   0b1010100101010000 }, /*   30   29   30   29   30   29   29   30   29   30   29   30        354
2011     */ { 00,   02,   03,   0b1011010010100000 }, /*   30   29   30   30   29   30   29   29   30   29   30   29        354
2012     */ { 04,   01,   23,   0b1011010101010000 }, /*   30   29   30   30   29   30   29   30   29   30   29   30   29   384
2013     */ { 00,   02,   10,   0b1010110101010000 }, /*   30   29   30   29   30   30   29   30   29   30   29   30        355
2014     */ { 09,   01,   31,   0b0101010110101000 }, /*   29   30   29   30   29   30   29   30   30   29   30   29   30   384
2015     */ { 00,   02,   19,   0b0100101110100000 }, /*   29   30   29   29   30   29   30   30   30   29   30   29        354
2016     */ { 00,   02,   08,   0b1010010110110000 }, /*   30   29   30   29   29   30   29   30   30   29   30   30        355
2017     */ { 06,   01,   28,   0b0101001010111000 }, /*   29   30   29   30   29   29   30   29   30   29   30   30   30   384
2018     */ { 00,   02,   16,   0b0101001010110000 }, /*   29   30   29   30   29   29   30   29   30   29   30   30        354
2019     */ { 00,   02,   05,   0b1010100100110000 }, /*   30   29   30   29   30   29   29   30   29   29   30   30        354
2020     */ { 04,   01,   25,   0b0111010010101000 }, /*   29   30   30   30   29   30   29   29   30   29   30   29   30   384
2021     */ { 00,   02,   12,   0b0110101010100000 }, /*   29   30   30   29   30   29   30   29   30   29   30   29        354
2022     */ { 00,   02,   01,   0b1010110101010000 }, /*   30   29   30   29   30   30   29   30   29   30   29   30        355
2023     */ { 02,   01,   22,   0b0100110110101000 }, /*   29   30   29   29   30   30   29   30   30   29   30   29   30   384
2024     */ { 00,   02,   10,   0b0100101101100000 }, /*   29   30   29   29   30   29   30   30   29   30   30   29        354
2025     */ { 06,   01,   29,   0b1010010101110000 }, /*   30   29   30   29   29   30   29   30   29   30   30   30   29   384
2026     */ { 00,   02,   17,   0b1010010011100000 }, /*   30   29   30   29   29   30   29   29   30   30   30   29        354
2027     */ { 00,   02,   06,   0b1101001001100000 }, /*   30   30   29   30   29   29   30   29   29   30   30   29        354
2028     */ { 05,   01,   26,   0b1110100100110000 }, /*   30   30   30   29   30   29   29   30   29   29   30   30   29   384
2029     */ { 00,   02,   13,   0b1101010100110000 }, /*   30   30   29   30   29   30   29   30   29   29   30   30        355
2030     */ { 00,   02,   03,   0b0101101010100000 }, /*   29   30   29   30   30   29   30   29   30   29   30   29        354
2031     */ { 03,   01,   23,   0b0110101101010000 }, /*   29   30   30   29   30   29   30   30   29   30   29   30   29   384
2032     */ { 00,   02,   11,   0b1001011011010000 }, /*   30   29   29   30   29   30   30   29   30   30   29   30        355
2033     */ { 11,   01,   31,   0b0100101011101000 }, /*   29   30   29   29   30   29   30   29   30   30   30   29   30   384
2034     */ { 00,   02,   19,   0b0100101011010000 }, /*   29   30   29   29   30   29   30   29   30   30   29   30        354
2035     */ { 00,   02,   08,   0b1010010011010000 }, /*   30   29   30   29   29   30   29   29   30   30   29   30        354
2036     */ { 06,   01,   28,   0b1101001001011000 }, /*   30   30   29   30   29   29   30   29   29   30   29   30   30   384
2037     */ { 00,   02,   15,   0b1101001001010000 }, /*   30   30   29   30   29   29   30   29   29   30   29   30        354
2038     */ { 00,   02,   04,   0b1101010100100000 }, /*   30   30   29   30   29   30   29   30   29   29   30   29        354
2039     */ { 05,   01,   24,   0b1101101010100000 }, /*   30   30   29   30   30   29   30   29   30   29   30   29   29   384
2040     */ { 00,   02,   12,   0b1011010110100000 }, /*   30   29   30   30   29   30   29   30   30   29   30   29        355
2041     */ { 00,   02,   01,   0b0101011011010000 }, /*   29   30   29   30   29   30   30   29   30   30   29   30        355
2042     */ { 02,   01,   22,   0b0100101011011000 }, /*   29   30   29   29   30   29   30   29   30   30   29   30   30   384
2043     */ { 00,   02,   10,   0b0100100110110000 }, /*   29   30   29   29   30   29   29   30   30   29   30   30        354
2044     */ { 07,   01,   30,   0b1010010010111000 }, /*   30   29   30   29   29   30   29   29   30   29   30   30   30   384
2045     */ { 00,   02,   17,   0b1010010010110000 }, /*   30   29   30   29   29   30   29   29   30   29   30   30        354
2046     */ { 00,   02,   06,   0b1010101001010000 }, /*   30   29   30   29   30   29   30   29   29   30   29   30        354
2047     */ { 05,   01,   26,   0b1011010100101000 }, /*   30   29   30   30   29   30   29   30   29   29   30   29   30   384
2048     */ { 00,   02,   14,   0b0110110100100000 }, /*   29   30   30   29   30   30   29   30   29   29   30   29        354
2049     */ { 00,   02,   02,   0b1010110110100000 }, /*   30   29   30   29   30   30   29   30   30   29   30   29        355
2050     */ { 03,   01,   23,   0b0101010110110000 }, /*   29   30   29   30   29   30   29   30   30   29   30   30   29   384
2051     */ { 00,   02,   11,   0b1001001101110000 }, /*   30   29   29   30   29   29   30   30   29   30   30   30        355
2052     */ { 08,   02,   01,   0b0100100101111000 }, /*   29   30   29   29   30   29   29   30   29   30   30   30   30   384
2053     */ { 00,   02,   19,   0b0100100101110000 }, /*   29   30   29   29   30   29   29   30   29   30   30   30        354
2054     */ { 00,   02,   08,   0b0110010010110000 }, /*   29   30   30   29   29   30   29   29   30   29   30   30        354
2055     */ { 06,   01,   28,   0b0110101001010000 }, /*   29   30   30   29   30   29   30   29   29   30   29   30   29   383
2056     */ { 00,   02,   15,   0b1110101001010000 }, /*   30   30   30   29   30   29   30   29   29   30   29   30        355
2057     */ { 00,   02,   04,   0b0110101010100000 }, /*   29   30   30   29   30   29   30   29   30   29   30   29        354
2058     */ { 04,   01,   24,   0b1010101101100000 }, /*   30   29   30   29   30   29   30   30   29   30   30   29   29   384
2059     */ { 00,   02,   12,   0b1010101011100000 }, /*   30   29   30   29   30   29   30   29   30   30   30   29        355
2060     */ { 00,   02,   02,   0b1001001011100000 }, /*   30   29   29   30   29   29   30   29   30   30   30   29        354
2061     */ { 03,   01,   21,   0b1100100101110000 }, /*   30   30   29   29   30   29   29   30   29   30   30   30   29   384
2062     */ { 00,   02,   09,   0b1100100101100000 }, /*   30   30   29   29   30   29   29   30   29   30   30   29        354
2063     */ { 07,   01,   29,   0b1101010010101000 }, /*   30   30   29   30   29   30   29   29   30   29   30   29   30   384
2064     */ { 00,   02,   17,   0b1101010010100000 }, /*   30   30   29   30   29   30   29   29   30   29   30   29        354
2065     */ { 00,   02,   05,   0b1101101001010000 }, /*   30   30   29   30   30   29   30   29   29   30   29   30        355
2066     */ { 05,   01,   26,   0b0101101010101000 }, /*   29   30   29   30   30   29   30   29   30   29   30   29   30   384
2067     */ { 00,   02,   14,   0b0101011010100000 }, /*   29   30   29   30   29   30   30   29   30   29   30   29        354
2068     */ { 00,   02,   03,   0b1010011011010000 }, /*   30   29   30   29   29   30   30   29   30   30   29   30        355
2069     */ { 04,   01,   23,   0b0101001011101000 }, /*   29   30   29   30   29   29   30   29   30   30   30   29   30   384
2070     */ { 00,   02,   11,   0b0101001011010000 }, /*   29   30   29   30   29   29   30   29   30   30   29   30        354
2071     */ { 08,   01,   31,   0b1010100101011000 }, /*   30   29   30   29   30   29   29   30   29   30   29   30   30   384
2072     */ { 00,   02,   19,   0b1010100101010000 }, /*   30   29   30   29   30   29   29   30   29   30   29   30        354
2073     */ { 00,   02,   07,   0b1011010010100000 }, /*   30   29   30   30   29   30   29   29   30   29   30   29        354
2074     */ { 06,   01,   27,   0b1011010101010000 }, /*   30   29   30   30   29   30   29   30   29   30   29   30   29   384
2075     */ { 00,   02,   15,   0b1010110101010000 }, /*   30   29   30   29   30   30   29   30   29   30   29   30        355
2076     */ { 00,   02,   05,   0b0101010110100000 }, /*   29   30   29   30   29   30   29   30   30   29   30   29        354
2077     */ { 04,   01,   24,   0b1010010111010000 }, /*   30   29   30   29   29   30   29   30   30   30   29   30   29   384
2078     */ { 00,   02,   12,   0b1010010110110000 }, /*   30   29   30   29   29   30   29   30   30   29   30   30        355
2079     */ { 00,   02,   02,   0b0101001010110000 }, /*   29   30   29   30   29   29   30   29   30   29   30   30        354
2080     */ { 03,   01,   22,   0b1010100100111000 }, /*   30   29   30   29   30   29   29   30   29   29   30   30   30   384
2081     */ { 00,   02,   09,   0b0110100100110000 }, /*   29   30   30   29   30   29   29   30   29   29   30   30        354
2082     */ { 07,   01,   29,   0b0111001010011000 }, /*   29   30   30   30   29   29   30   29   30   29   29   30   30   384
2083     */ { 00,   02,   17,   0b0110101010100000 }, /*   29   30   30   29   30   29   30   29   30   29   30   29        354
2084     */ { 00,   02,   06,   0b1010110101010000 }, /*   30   29   30   29   30   30   29   30   29   30   29   30        355
2085     */ { 05,   01,   26,   0b0100110110101000 }, /*   29   30   29   29   30   30   29   30   30   29   30   29   30   384
2086     */ { 00,   02,   14,   0b0100101101100000 }, /*   29   30   29   29   30   29   30   30   29   30   30   29        354
2087     */ { 00,   02,   03,   0b1010010101110000 }, /*   30   29   30   29   29   30   29   30   29   30   30   30        355
2088     */ { 04,   01,   24,   0b0101001001110000 }, /*   29   30   29   30   29   29   30   29   29   30   30   30   29   383
2089     */ { 00,   02,   10,   0b1101000101100000 }, /*   30   30   29   30   29   29   29   30   29   30   30   29        354
2090     */ { 08,   01,   30,   0b1110100100110000 }, /*   30   30   30   29   30   29   29   30   29   29   30   30   29   384
2091     */ { 00,   02,   18,   0b1101010100100000 }, /*   30   30   29   30   29   30   29   30   29   29   30   29        354
2092     */ { 00,   02,   07,   0b1101101010100000 }, /*   30   30   29   30   30   29   30   29   30   29   30   29        355
2093     */ { 06,   01,   27,   0b0110101101010000 }, /*   29   30   30   29   30   29   30   30   29   30   29   30   29   384
2094     */ { 00,   02,   15,   0b0101011011010000 }, /*   29   30   29   30   29   30   30   29   30   30   29   30        355
2095     */ { 00,   02,   05,   0b0100101011100000 }, /*   29   30   29   29   30   29   30   29   30   30   30   29        354
2096     */ { 04,   01,   25,   0b1010010011101000 }, /*   30   29   30   29   29   30   29   29   30   30   30   29   30   384
2097     */ { 00,   02,   12,   0b1010001011010000 }, /*   30   29   30   29   29   29   30   29   30   30   29   30        354
2098     */ { 00,   02,   01,   0b1101000101010000 }, /*   30   30   29   30   29   29   29   30   29   30   29   30        354
2099     */ { 02,   01,   21,   0b1101100100101000 }, /*   30   30   29   30   30   29   29   30   29   29   30   29   30   384
2100     */ { 00,   02,   09,   0b1101010100100000 }, /*   30   30   29   30   29   30   29   30   29   29   30   29        354
         */ };

        internal override int MinCalendarYear => MinLunisolarYear;

        internal override int MaxCalendarYear => MaxLunisolarYear;

        internal override DateTime MinDate => s_minDate;

        internal override DateTime MaxDate => s_maxDate;

        internal override EraInfo[]? CalEraInfo => null;

        internal override int GetYearInfo(int lunarYear, int index)
        {
            if (lunarYear < MinLunisolarYear || lunarYear > MaxLunisolarYear)
            {
                throw new ArgumentOutOfRangeException("year", lunarYear, SR.Format(SR.ArgumentOutOfRange_Range, MinLunisolarYear, MaxLunisolarYear));
            }

            return s_yinfo[lunarYear - MinLunisolarYear, index];
        }

        internal override int GetYear(int year, DateTime time)
        {
            return year;
        }

        internal override int GetGregorianYear(int year, int era)
        {
            if (era != CurrentEra && era != ChineseEra)
            {
                throw new ArgumentOutOfRangeException(nameof(era), era, SR.ArgumentOutOfRange_InvalidEraValue);
            }
            if (year < MinLunisolarYear || year > MaxLunisolarYear)
            {
                throw new ArgumentOutOfRangeException(nameof(year), year, SR.Format(SR.ArgumentOutOfRange_Range, MinLunisolarYear, MaxLunisolarYear));
            }

            return year;
        }

        public ChineseLunisolarCalendar()
        {
        }

        public override int GetEra(DateTime time)
        {
            CheckTicksRange(time.Ticks);
            return ChineseEra;
        }

        internal override CalendarId ID => CalendarId.CHINESELUNISOLAR;

        internal override CalendarId BaseCalendarID
        {
            get
            {
                //Use CAL_GREGORIAN just to get CurrentEraValue as 1 since we do not have data under the ID CAL_ChineseLunisolar yet
                return CalendarId.GREGORIAN;
            }
        }

        public override int[] Eras => new int[] { ChineseEra };
    }
}
