// Copyright(c) 2026-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _FDP_H_
#define      _FDP_H_

// to also use ',' as decimal point -->  #define COMMA_AS_DP 
#define COMMA_AS_DP

class wxString;

class Fdp
{   // The FixedDecimalPoint class: a 'long long' with implied decimal point
    // Operators: multiply, divide, add, subtract and all compare methods
    // Operands can be a mix of Fdp, int, long, long long
    // conversion from/to string and int-types
public:
    typedef unsigned int UINT;
    typedef long long    LL;
    enum class StringType
    {   // how to convert value to string
          LONG      // display dp and all fraction digits      --> 123.010 --> '123.010'
        , SHORT     // don't display trailing '0' and '.'      --> 123.000 --> '123'     123.100 --> '123.1'
        , EXTRA     // if fraction zero, use spaces, else LONG --> 123.000 --> '123    ' 123.100 --> '123.100'
    };

                Fdp         ()      : m_value(0)            {}
                Fdp         (const Fdp& other) : m_value(other.m_value) {}
                // cppcheck-suppress noExplicitConstructor
                Fdp         (LL val): m_value(val * FACTOR) {}  // Can't use explicit: 'Fdp var = 3;' will give error
                // cppcheck-suppress noExplicitConstructor
                Fdp         (const wxString& val);
                // cppcheck-suppress noExplicitConstructor
//     explicit Fdp         (const char* val) :Fdp(wxString(val)) {}; // error for: Fdp var; var = 0; use: var = 0LL;

           Fdp  FromProcent2(int procent) { m_value = procent*DIVISOR[2]; return *this;}    // 3 --> 0.030
    static Fdp  FromProcent (int procent)       // 3 --> 0.030
                { // create an Fdp from % value: xx% -> Fdp(0.xx)
                  Fdp tmp;
                  tmp.m_value=procent*DIVISOR[2];   // dp == 2
                  return tmp;
                }  // FromProcent()
    wxString AsString  () const {return ToString(0, StringType::SHORT); }
    wxString AsString1 () const {return ToString(1, StringType::SHORT); }
    wxString AsString1F() const {return ToString(1, StringType::LONG);  }
    wxString AsString1E() const {return ToString(1, StringType::EXTRA); }

    wxString AsString2 () const {return ToString(2, StringType::SHORT); }
    wxString AsString2F() const {return ToString(2, StringType::LONG);  }
    wxString AsString2E() const {return ToString(2, StringType::EXTRA); }

    wxString AsString3 () const {return ToString(3, StringType::SHORT); }
    wxString AsString3F() const {return ToString(3, StringType::LONG);  }
    wxString AsString3E() const {return ToString(3, StringType::EXTRA); }

        long AsLong    () const {return DoGet(0);}   // get value as x
        long AsLong1   () const {return DoGet(1);}   // get value as x.1
        long AsLong2   () const {return DoGet(2);};  // get value as x.12
        long AsLong3   () const {return DoGet(3);}   // get value as x.123
         Fdp& Round    (UINT a_dp) {m_value += Sign()*ROUND[a_dp];m_value /= DIVISOR[a_dp]; m_value *= DIVISOR[a_dp]; return *this;} // round value
         Fdp& Trunc    (UINT a_dp) {                             ;m_value /= DIVISOR[a_dp]; m_value *= DIVISOR[a_dp]; return *this;} // truncate value

    explicit operator bool()                      const{return m_value != 0;}
           Fdp& operator+=(        const Fdp& rhs)     {m_value +=rhs.m_value; return *this;}
           Fdp& operator-=(        const Fdp& rhs)     {m_value -=rhs.m_value; return *this;}
           Fdp& operator+=(                LL rhs)     {m_value +=rhs*FACTOR;  return *this;}
           Fdp& operator-=(                LL rhs)     {m_value -=rhs*FACTOR;  return *this;}
           Fdp  operator- ()                      const{Fdp tmp; tmp.m_value = -m_value; return tmp;}
           Fdp  operator+ (        const Fdp& rhs)const{Fdp tmp; tmp.m_value = m_value + rhs.m_value; return tmp;}
           Fdp  operator+ (                LL rhs)const{Fdp tmp; tmp.m_value = m_value + rhs*FACTOR;  return tmp;}
    friend Fdp  operator+ (LL lhs, const Fdp& rhs)     {Fdp tmp; tmp.m_value = lhs*FACTOR + rhs.m_value; return tmp;}
           Fdp  operator- (        const Fdp& rhs)const{Fdp tmp; tmp.m_value = m_value - rhs.m_value; return tmp;}
           Fdp  operator- (                LL rhs)const{Fdp tmp; tmp.m_value = m_value - rhs*FACTOR;  return tmp;}
    friend Fdp  operator- (LL lhs, const Fdp& rhs){Fdp tmp; tmp.m_value = lhs*FACTOR - rhs.m_value; return tmp;}
           Fdp  operator* (        const Fdp& rhs)const{Fdp tmp; tmp.m_value = (m_value * rhs.m_value)/ FACTOR; return tmp;}
           Fdp  operator* (                LL rhs)const{Fdp tmp; tmp.m_value = m_value * rhs;  return tmp;}
    friend Fdp  operator* (LL lhs, const Fdp& rhs)     {Fdp tmp; tmp.m_value = lhs*rhs.m_value; return tmp;}
           Fdp  operator/ (        const Fdp& rhs)const{Fdp tmp; tmp.m_value = (m_value * FACTOR)/rhs.m_value; return tmp;}
           Fdp  operator/ (                LL rhs)const{Fdp tmp; tmp.m_value = m_value / rhs;  return tmp;}
    friend Fdp  operator/ (LL lhs, const Fdp& rhs)     {Fdp tmp; tmp.m_value = (FACTOR*FACTOR*lhs)/rhs.m_value; return tmp;}
           bool operator==(        const Fdp& rhs)const{return m_value == rhs.m_value;}
           bool operator==(                LL rhs)const{return m_value == rhs*FACTOR ;}
           bool operator< (        const Fdp& rhs)const{return m_value <  rhs.m_value;}
           bool operator< (                LL rhs)const{return m_value <  rhs*FACTOR ;}
           bool operator> (        const Fdp& rhs)const{return m_value >  rhs.m_value;}
           bool operator> (                LL rhs)const{return m_value >  rhs*FACTOR ;}
           bool operator!=(                LL rhs)const{return !(*this == rhs);}
           bool operator!=(        const Fdp& rhs)const{return !(*this == rhs);}
           bool operator<=(        const Fdp& rhs)const{return !(*this >  rhs);}
           bool operator<=(                LL rhs)const{return !(*this >  rhs);}
           bool operator>=(        const Fdp& rhs)const{return !(*this <  rhs);}
           bool operator>=(                LL rhs)const{return !(*this <  rhs);}
    friend bool operator==(LL lhs, const Fdp& rhs)     {return lhs*FACTOR == rhs.m_value;}
    friend bool operator< (LL lhs, const Fdp& rhs)     {return lhs*FACTOR  < rhs.m_value;}
    friend bool operator> (LL lhs, const Fdp& rhs)     {return lhs*FACTOR  > rhs.m_value;}
    friend bool operator!=(LL lhs, const Fdp& rhs)     {return !(lhs == rhs);}
    friend bool operator<=(LL lhs, const Fdp& rhs)     {return !(lhs >  rhs);}
    friend bool operator>=(LL lhs, const Fdp& rhs)     {return !(lhs <  rhs);}

//    Fdp& operator=(LL lhs) { Fdp tmp; tmp.m_value = lhs*FACTOR; return tmp; } // returning ref to local var???
    Fdp& operator=(const Fdp& other) { if ( this != &other ) m_value = other.m_value; return *this; }

private:
    inline static const LL   FACTOR           = 1000LL;     // for 3 decimals: 2 for real and 1 for rounding
    inline static const LL   FRACTION_DIGITS  = 3;          // cnv from string: use this value of fractional digits
    inline static const LL   DIVISOR[]        = {FACTOR/1, FACTOR/10, FACTOR/100, FACTOR/1000};     // for dp 0, 1, 2, 3
    inline static const LL   ROUND  []        = {FACTOR/2, FACTOR/20, FACTOR/200, FACTOR/2000};     // round-values for dp 0, 1, 2, 3

    wxString  ToString(UINT dp, StringType type) const;
    int       Sign    ()                   const { return (m_value < 0) ? -1 : +1; }
    LL        DoGet   (UINT a_dp)                const
                {   // get the decimal presentation as LL
                    LL value = m_value + Sign()*ROUND[a_dp]; // prepare rounding
                    value /=  DIVISOR[a_dp];                        // get rounded value
                    return value;
                }   // DoGet()

    LL   m_value;	// the only member variable as fixed decimal point xxx.yyy
};  // class Fdp

#endif
