#include <wx/app.h>
//#include <wx/cmdline.h>
#include <wx/msgdlg.h>

#include "fdp.h"

#define STANDALONE_TEST 1   /* set to 1 for standalone testing: main defined*/
#define TEST_CNV        1   /* messagebox if conversion wxString::ToLongLong() gives an error */

#if STANDALONE_TEST == 0
#include "mylog.h"
#endif

#define FMT wxString::Format

Fdp::Fdp(const wxString& a_val)
{   // "1234.5678"  "-123" "123."  ".123" "-.2"
    // NB, take max FRACTION_DIGITS digits after dp and DONOT round here! Round when string requested!
    // fe: '0.1499999' should give '0.1' for .ToString1(), so store : 149LL
    // [space][+|-][<digits>][[.|,][<digits>]]
    m_value = 0;
    // for now, assume 'correct' formatted value! "12.3"
    wxString    work(a_val); work.Trim(false).Trim(true);   // remove leading/trailing white-space
    auto        dp       = work.find('.');

    #ifdef COMMA_AS_DP
    if ( dp == wxString::npos )
        dp = work.find(',');
    #endif

    LL mpFactor;            // multiplication factor if we (don't) have a dp
    if ( dp == wxString::npos )
    {
        mpFactor = FACTOR;  // no fraction, so multiply endresult asif we had an x-digit fraction
    }
    else
    {   // 123.456xxxxx  size=7 dp=3  dif=size-dp=4 --> if dif < 3+1 -->add '000'
        if ( work.size() <= dp + FRACTION_DIGITS )
            work += wxString('0', FRACTION_DIGITS); // make sure we have atleast FRACTION_DIGITS digits after the dp
        work.erase(dp, 1);                          // remove the '.'
        work.erase(dp + FRACTION_DIGITS);           //   and the superfluous fraction digits: now exactly FRACTION_DIGITS digits
        mpFactor = 1;                               // we have max fraction digits, so no multiplication needed
    }
    /*
        errno = 0;
        char*    end1;
        wchar_t* end2;
        long long var1 = strtoll("1234", &end1, 10);
        long long var2 = wcstoll(work.c_str(), &end2, 10);
        bool bErr1 = (errno == ERANGE);
        bool bErr2 = (var1 == LLONG_MAX) || (var1 == LLONG_MIN);
    */
    auto bOk = work.ToLongLong(&m_value, 10);
    if ( !bOk && !work.IsEmpty() )  // stupid wx gives error if string is empty...
    {
        wxString error = FMT("Conversion from '%s' to longlong failed!", work);
        #if TEST_CNV == 1
            wxMessageBox(error,"ERROR", wxICON_INFORMATION);
        #endif
        #if STANDALONE_TEST == 0
            MyLogError("%s", error);
        #endif
        m_value = 0;        // apparently a bad formatted string! Set value to 0
    }

    m_value *= mpFactor;    // now we have a value with FRACTION_DIGITS decimals
}   // Fdp::Fdp(wxString)

wxString Fdp::ToString(UINT a_dp, StringType a_type) const
{
    bool bSign  = m_value < 0;
    LL   value  = ROUND[a_dp] + (bSign ? -m_value : +m_value);  // prepare absolute rounded value
    value      /= DIVISOR[a_dp];                                // remove unwanted digits at the end
    bSign      &= value != 0;                                   // update sign: value COULD be zero after division
    LL fraction = value % (FACTOR / DIVISOR[a_dp]);             // determine the fraction

    wxString result;        // build result in reverse order
    for ( UINT count = 0; count <= a_dp || value; ++count )
    {   // need/want atleast a_dp+1 digits
        result += static_cast<unsigned char>('0' + (value % 10));
        value /= 10;
    }
    if ( bSign ) result += '-';                 // add sign if needed
    result.insert(a_dp, '.');                   // and insert dp
    std::reverse(result.begin(), result.end()); // back to 'normal' order
    switch ( a_type )
    {   // check what 'op' we want
        case StringType::SHORT: // remove trailing '0' and '.' as much as possible
            while ( *result.rbegin() == '0' ) result.RemoveLast();
            if    ( *result.rbegin() == '.' ) result.RemoveLast();
            break;
        case StringType::EXTRA: // replace fraction and '.' with spaces if fraction == zero
            if ( fraction == 0 )
                result.replace (result.size()-a_dp-1, a_dp+1ULL, a_dp+1ULL, ' ');
            break;
        case StringType::LONG:  // do nothing, we want all...
            break;
    }

    return result;
}   // Fdp::ToString()

#if STANDALONE_TEST == 1
#include <iostream>
#include <iomanip>      // std::setw

void Print(const Fdp& val)
{
    std::cout << std::setw(25) << val.AsString3F() << '\n';
}   // Print()


void Test(const char* msg, const Fdp& value)
{
    std::cout << std::right << std::setw(25) << msg << std::right << std::setw(10) << value.AsString2F();
    Print(value); 
}   // Test()

void ShowAll(const char* input)
{
    Fdp value(input);
    std::cout << std::setw(10) << input            << " " ;
    std::cout << std::setw(10) << value.AsString1 () << " " ;
    std::cout << std::setw(10) << value.AsString1E() << " " ;
    std::cout << std::setw(10) << value.AsString2 () << " " ;
    std::cout << std::setw(10) << value.AsString2E() << " " ;
    std::cout << std::setw(10) << value.AsString3 () << " " ;
    std::cout << std::setw(10) << value.AsString3E() << " " ;
    std::cout << std::setw(10) << value.AsString2F() << "\n" ;
}   // ShowAll()

void ShowBool(const char* pTest, bool bResult)
{
    std::cout << std::setw(20) << pTest << ": " << (bResult ? "true" : "false") << "\n";
}   // ShowBool()

volatile long long procent(75);
int main()
{
    Fdp         n200("  12  ");     (void)n200;
    Fdp         n201 = Fdp("12");   (void)n201;
    wxString    n202 = "123";
    Fdp         n203 = n202;        (void)n203;
    n203 = Fdp("");                 (void)n203;
    Fdp         n204 = 0;           (void)n204;
    n204 = 0;                       (void)n204;
    std::cout << "   input      String1   String1E    String2   String2E    String3   String3E   String2F \n" ;
    ShowAll("123.45678" );
    ShowAll("123,45678" );
    ShowAll("invalid"   );
    ShowAll("123"       );
    ShowAll("100.0"     );
    ShowAll(".567"      );
    ShowAll("-.12"      );
    ShowAll("-.012"     );
    ShowAll("-.1"       );
    ShowAll("-.0019"    );
    ShowAll("-1.0019"   );
    ShowAll("-."        );
    ShowAll("-3.1206"   );
    ShowAll("-3.14999"  );

    std::cout << '\n';
    Fdp n0(2); n0=n0/3; Test(" n0         :   0.666", n0);
    auto size = sizeof(n0); (void)size;
    n0=n0;
    Fdp n1(3);          Test(" n1(3)      :   3    ", n1);
    n1 = n1 + 4;        Test(" n1=n1+4    :   7    ", n1);
    Fdp n2 = n1;        Test(" n2=n1      :   7    ", n2);
    n2 = 3 + n1;        Test(" n2=3+n1    :  10    ", n2);
    Fdp n3 = n1 - 2;    Test(" n3=n1-2    :   5    ", n3);
    n1=n3-n2;           Test(" n1=n3-n2   :  -5    ", n1);
    n1 = n1*n2;         Test(" n1=n1*n2   : -50    ", n1);
    n2=n1*3;            Test(" n2=n1*3    :-150    ", n2);
    n2 = n2/3;          Test(" n2=n2/3    : -50    ", n2);
    n1 = n1/n2;         Test(" n1=n1/n2   :   1    ", n1);
    Fdp n10(n1);        Test("n10(n1)     :   1    ", n10);
    Fdp n11 = 6/Fdp(1); Test("n11=6/Fdp(1):   6    ", n11);

    std::cout << '\n';
    ShowBool( "n11 != 6"    , n11 != 6      );
    ShowBool( "n11 <= 6"    , n11 <= 6      );
    ShowBool( "n11 >= 6"    , n11 >= 6      );
    ShowBool( "n11 != n10"  , n11 != n10    );
    ShowBool( "n11 <= n10"  , n11 <= n10    );
    ShowBool( "n11 >= n10"  , n11 >= n10    );
    ShowBool( "3 > n11"     , 3 > n11       );
    ShowBool( "3 >= n11"    , 3 >= n11      );
    ShowBool( "3 == n11"    , 3 == n11      );
    ShowBool( "3 < n11"     , 3 < n11       );
    ShowBool( "3 <= n11"    , 3 <= n11      );
    ShowBool( "3 != n11"    , 3 != n11      );
    ShowBool( "Fdp(3)*Fdp(6) == 18", Fdp(3) * Fdp(6) == 18 );
    ShowBool( "18 == Fdp(3)*Fdp(6)", 18 == Fdp(3) * Fdp(6) );

    Fdp n30("123.45678");
    long long v1 = n30.AsLong1(); (void)v1;
    long long v2 = n30.AsLong2(); (void)v2;
    long long v3 = n30.AsLong3(); (void)v3;
    n30 = 0-n30;
    n30 = -n30;
    v1 = n30.AsLong1(); (void)v1;
    v2 = n30.AsLong2(); (void)v2;
    v3 = n30.AsLong3(); (void)v3;

    std::cout << '\n';
    auto n40 = Fdp().FromProcent2(70);   Test("Fdp().FromProcent2(70)", n40);
    auto n41 = Fdp::FromProcent(70);     Test("Fdp::FromProcent(70)"  , n41);
    auto n43 = Fdp::FromProcent(procent);Test("Fdp::FromProcent(pr)"  , n43);
    auto n42 = 3*Fdp::FromProcent(70);   Test("3*Fdp::FromProcent(70)", n42);

    Fdp n100 (3); (void)n100;
    Fdp n101 = 3; (void)n101;
    n101 = 4;     (void)n101;

    long n55 = Fdp("-1.2567").AsLong1();
    Fdp n56 = n55; (void)n56;
    int n57 = Fdp("-1.2567").AsLong1();
    Fdp n58 = n57; (void)n58;
    Fdp::LL n50 = Fdp("-1.2567").AsLong1(); (void) n50;
    Fdp::LL n51 = Fdp("-1.2567").AsLong2(); (void) n51;
    Fdp::LL n52 = Fdp("-1.2567").AsLong3(); (void) n52;
    long n53 = n50; (void) n53;
    if (n53 )
        n53 = 0; (void)n53;
    return 0;
}   // main()
#endif
