/*
This Source Code Form is subject to the
terms of the Mozilla Public License, v.
2.0. If a copy of the MPL was not
distributed with this file, You can
obtain one at
http://mozilla.org/MPL/2.0/.
*/
/*
MaSzyna EU07 - SPKS
Brakes. Oerlikon ESt.
Copyright (C) 2007-2014 Maciej Cierniak
*/

#include "Oerlikon_ESt.h"

double d2A(double d)
{   double result;
  d2A =( d*d )*0.7854*1.0/1000;
return result;
}

// ------ RURA ------

double TRura::P()
{   double result;
  P =Next.P;
return result;
}

void TRura::Update(double dt)
{
  Next.Flow( dVol );
  dVol =0;
}

// ------ PRZECIWPOSLIG ------

void TPrzeciwposlizg::SetPoslizg(bool flag)
{
  Poslizg =flag;
}

void TPrzeciwposlizg::Update(double dt)
{
  if(  ( Poslizg ) )
   {
    BrakeRes.Flow( dVol );
    Next.Flow( PF( Next.P, 0, d2A( 10 ) )*dt );
   }
  else
    Next.Flow( dVol );
  dVol =0;
}


// ------ PRZEKLADNIK ------

void TPrzekladnik::Update(double dt)
{ double BCP; double BVP; double dV;


  BCP =Next.P;
  BVP =BrakeRes.P;

  if( ( BCP>P ) )
   dV =-PFVd( BCP, 0, d2A( 10 ), P )*dt;else
  if( ( BCP<P ) )
   dV =PFVa( BVP, BCP, d2A( 10 ), P )*dt;else dV =0;

  Next.Flow( dV );
  if(  dV>0 )
  BrakeRes.Flow( -dV );

}

// ------ PRZEKLADNIK RAPID ------

void TRapid::SetRapidParams(double mult,  double size)
{
  RapidMult =mult;
  RapidStatus =false;
  if(  ( size>0.1 ) ) //dopasowywanie srednicy przekladnika
   {
    DN =d2A( size*0.4 );
    DL =d2A( size*0.4 );
   }
  else
   {
    DN =d2A( 5 );
    DL =d2A( 5 );
   }
}

void TRapid::SetRapidStatus(bool rs)
{
  RapidStatus =rs;
}

void TRapid::Update(double dt)
{ double BCP; double BVP; double dV; double ActMult;

  BVP =BrakeRes.P;
  BCP =Next.P;

  if( ( RapidStatus ) )
   {
    ActMult =RapidMult;
   }
  else
   {
    ActMult =1;
   }

  if( ( BCP*RapidMult>P*ActMult ) )
   dV =-PFVd( BCP, 0, DL, P*ActMult*1.0/RapidMult )*dt;else
  if( ( BCP*RapidMult<P*ActMult ) )
   dV =PFVa( BVP, BCP, DN, P*ActMult*1.0/RapidMult )*dt;else dV =0;

  Next.Flow( dV );
  if(  dV>0 )
  BrakeRes.Flow( -dV );

}

// ------ PRZEK�ADNIK CI�G�Y ------

void TPrzekCiagly::SetMult(double m)
{
  mult =m;
}

void TPrzekCiagly::Update(double dt)
{ double BCP; double BVP; double dV;

  BVP =BrakeRes.P;
  BCP =Next.P;

  if( ( BCP>P*mult ) )
   dV =-PFVd( BCP, 0, d2A( 8 ), P*mult )*dt;else
  if( ( BCP<P*mult ) )
   dV =PFVa( BVP, BCP, d2A( 8 ), P*mult )*dt;else dV =0;

  Next.Flow( dV );
  if(  dV>0 )
  BrakeRes.Flow( -dV );

}

// ------ PRZEK�ADNIK CI�G�Y ------

void TPrzek_PZZ::SetLBP(double P)
{
  LBP =P;
}

void TPrzek_PZZ::Update(double dt)
{ double BCP; double BVP; double dV; double Pgr;

  BVP =BrakeRes.P;
  BCP =Next.P;

  Pgr =Max0R( LBP, P );

  if( ( BCP>Pgr ) )
   dV =-PFVd( BCP, 0, d2A( 8 ), Pgr )*dt;else
  if( ( BCP<Pgr ) )
   dV =PFVa( BVP, BCP, d2A( 8 ), Pgr )*dt;else dV =0;

  Next.Flow( dV );
  if(  dV>0 )
  BrakeRes.Flow( -dV );

}

// ------ PRZECIWPOSLIG ------

void TPrzekED::SetP(double P)
{
  MaxP =P;
}

void TPrzekED::Update(double dt)
{
  if(  Next.P>MaxP )
   {
    BrakeRes.Flow( dVol );
    Next.Flow( PFVd( Next.P, 0, d2A( 10 )*dt, MaxP ) );
   }
  else
    Next.Flow( dVol );
  dVol =0;
}


// ------ OERLIKON EST NA BOGATO ------

double TNESt3::GetPF(double PP, double dt, double Vel)     //przeplyw miedzy komora wstepna i PG
{   double result; double dV; double dV1; double temp;
    double VVP; double BVP; double BCP; double CVP; double MPP; double nastG;
    unsigned char i;

  BVP =BrakeRes.P;
  VVP =ValveRes.P;
//  BCP:=BrakeCyl.P;
  BCP =Przekladniki[ 1 ].P;
  CVP =CntrlRes.P-0.0;
  MPP =Miedzypoj.P;
  dV1 =0;

  nastG =( BrakeDelayFlag && bdelay_G );

//sprawdzanie stanu
  CheckState( BCP , dV1 );
  CheckReleaser( dt );

//luzowanie
  if( ( BrakeStatus && b_hld )==b_off )
   dV =PF( 0, BCP, Nozzles[ dTO ]*nastG+( 1-nastG )*Nozzles[ dOO ] )*dt*( 0.1+4.9*Min0R( 0.2, BCP-( ( CVP-0.05-VVP )*BVM+0.1 ) ) );else dV =0;
//  BrakeCyl.Flow(-dV);
  Przekladniki[ 1 ].Flow( -dV );
  if( ( ( BrakeStatus && b_on )==b_on )&&( Przekladniki[ 1 ].P*HBG300<MaxBP ) )
   dV =PF( BVP, BCP, Nozzles[ dTN ]*( nastG+2*unsigned char( BCP<Podskok ) )+Nozzles[ dON ]*( 1-nastG ) )*dt*( 0.1+4.9*Min0R( 0.2, ( CVP-0.05-VVP )*BVM-BCP ) );else dV =0;
//  BrakeCyl.Flow(-dV);
  Przekladniki[ 1 ].Flow( -dV );
  BrakeRes.Flow( dV );

  { long i_end = 4 ; for( i = 1 ; i < i_end ; ++i )
   {
    Przekladniki[ i ].Update( dt );
    if(  ( Przekladniki[ i ] is TRapid ) )
     {
      RapidStatus =( ( ( BrakeDelayFlag && bdelay_R )==bdelay_R ) && ( ( abs( Vel )>70 ) || ( ( RapidStatus ) && ( abs( Vel )>50 ) ) || ( RapidStaly ) ) );
      ( Przekladniki[ i ] as TRapid ).SetRapidStatus( RapidStatus );
     }
    else
    if(  ( Przekladniki[ i ] is TPrzeciwposlizg ) )
      ( Przekladniki[ i ] as TPrzeciwposlizg ).SetPoslizg( ( BrakeStatus && b_asb )==b_asb );else
    if(  ( Przekladniki[ i ] is TPrzekED ) )
      if(  ( Vel<-15 ) )
       ( Przekladniki[ i ] as TPrzekED ).SetP( 0 );else ( Przekladniki[ i ] as TPrzekED ).SetP( MaxBP*3 );else
    if(  ( Przekladniki[ i ] is TPrzekCiagly ) )
      ( Przekladniki[ i ] as TPrzekCiagly ).SetMult( LoadC );else
    if(  ( Przekladniki[ i ] is TPrzek_PZZ ) )
      ( Przekladniki[ i ] as TPrzek_PZZ ).SetLBP( LBP );
   }}


//przeplyw testowy miedzypojemnosci
  dV =PF( MPP, VVP, BVs( BCP ) )+PF( MPP, CVP, CVs( BCP ) );
  if( ( MPP-0.05>BVP ) )
    dV =dV+PF( MPP-0.05, BVP, Nozzles[ dPT ]*nastG+( 1-nastG )*Nozzles[ dPO ] );
  if(  MPP>VVP ) dV =dV+PF( MPP, VVP, d2A( 5 ) );
  Miedzypoj.Flow( dV*dt*0.15 );


//przeplyw ZS <-> PG
  temp =CVs( BCP );
  dV =PF( CVP, MPP, temp )*dt;
  CntrlRes.Flow( +dV );
  ValveRes.Flow( -0.02*dV );
  dV1 =dV1+0.98*dV;

//przeplyw ZP <-> MPJ
  if( ( MPP-0.05>BVP ) )
   dV =PF( BVP, MPP-0.05, Nozzles[ dPT ]*nastG+( 1-nastG )*Nozzles[ dPO ] )*dt;else dV =0;
  BrakeRes.Flow( dV );
  dV1 =dV1+dV*0.98;
  ValveRes.Flow( -0.02*dV );
//przeplyw PG <-> rozdzielacz
  dV =PF( PP, VVP, 0.005 )*dt;    //0.01
  ValveRes.Flow( -dV );


  ValveRes.Act;
  BrakeCyl.Act;
  BrakeRes.Act;
  CntrlRes.Act;
  Miedzypoj.Act;
  Przekladniki[ 1 ].Act;
  Przekladniki[ 2 ].Act;
  Przekladniki[ 3 ].Act;
  GetPF =dV-dV1;
return result;
}


void TNESt3::EStParams(double i_crc)                 //parametry charakterystyczne dla ESt
{

}


void TNESt3::Init(double PP, double HPP, double LPP, double BP,  unsigned char BDF)
{
   ValveRes.CreatePress( 1*PP );
   BrakeCyl.CreatePress( 1*BP );
   BrakeRes.CreatePress( 1*PP );
   CntrlRes =TReservoir.Create;
   CntrlRes.CreateCap( 15 );
   CntrlRes.CreatePress( 1*HPP );
   BrakeStatus =unsigned char( BP>1 )*1;
   Miedzypoj =TReservoir.Create;
   Miedzypoj.CreateCap( 5 );
   Miedzypoj.CreatePress( PP );

   BVM =1*1.0/( HPP-0.05-LPP )*MaxBP;

   BrakeDelayFlag =BDF;

   Zamykajacy =false;

   if(  !( ( FM is TDisk1 ) || ( FM is TDisk2 ) ) )   //jesli zeliwo to schodz
     RapidStaly =false;else
     RapidStaly =true;

}


double TNESt3::GetCRP()
{   double result;
  GetCRP =CntrlRes.P;
//  GetCRP:=Przekladniki[1].P;
//  GetCRP:=Miedzypoj.P;
return result;
}


void TNESt3::CheckState(double BCP,   double & dV1) //glowny przyrzad rozrzadczy
{ double VVP; double BVP; double CVP; double MPP;

  BVP =BrakeRes.P;  //-> tu ma byc komora rozprezna
  VVP =ValveRes.P;
  CVP =CntrlRes.P;
  MPP =Miedzypoj.P;

  if( ( BCP<0.25 )&&( VVP+0.08>CVP ) ) Przys_blok =false;

//sprawdzanie stanu
// if ((BrakeStatus and 1)=1)and(BCP>0.25)then
   if( ( VVP+0.01+BCP*1.0/BVM<CVP-0.05 )&&( Przys_blok ) )
     BrakeStatus =( BrakeStatus || 3 ) //hamowanie stopniowe;else if( ( VVP-0.01+( BCP-0.1 )*1.0/BVM>CVP-0.05 ) )
     BrakeStatus =( BrakeStatus && 252 ) //luzowanie;else if( ( VVP+BCP*1.0/BVM>CVP-0.05 ) )
     BrakeStatus =( BrakeStatus && 253 ) //zatrzymanie napelaniania;else if( ( VVP+( BCP-0.1 )*1.0/BVM<CVP-0.05 )&&( BCP>0.25 ) ) //zatrzymanie luzowania
     BrakeStatus =( BrakeStatus || 1 );

 if(  ( BrakeStatus && 1 )==0 )
   SoundFlag =SoundFlag || sf_CylU;

 if( ( VVP+0.10<CVP )&&( BCP<0.25 ) )    //poczatek hamowania
   if(  ( !Przys_blok ) )
    {
     ValveRes.CreatePress( 0.1*VVP );
     SoundFlag =SoundFlag || sf_Acc;
     ValveRes.Act;
     Przys_blok =true;
    }


 if( ( BCP>0.5 ) )
   Zamykajacy =true;else if( ( VVP-0.6<MPP ) )
   Zamykajacy =false;

}


void TNESt3::CheckReleaser(double dt) //odluzniacz
{
  double VVP; double CVP;

  VVP =ValveRes.P;
  CVP =CntrlRes.P;

//odluzniacz automatyczny
 if( ( BrakeStatus && b_rls==b_rls ) )
  {
   CntrlRes.Flow( +PF( CVP, 0, 0.02 )*dt );
   if( ( CVP<VVP+0.3 )||( !autom ) )
     BrakeStatus =BrakeStatus && 247;
  }
}


double TNESt3::CVs(double BP)      //napelniacz sterujacego
{   double result; double CVP; double MPP;

  CVP =CntrlRes.P;
  MPP =Miedzypoj.P;

//przeplyw ZS <-> PG
  if( ( MPP<CVP-0.17 ) )
    CVs =0;else
  if( ( MPP>CVP-0.08 ) )
    CVs =Nozzles[ dSd ];else
    CVs =Nozzles[ dSm ];
return result;
}


double TNESt3::BVs(double BCP)     //napelniacz pomocniczego
{   double result; double CVP; double MPP;

  CVP =CntrlRes.P;
  MPP =Miedzypoj.P;

//przeplyw ZP <-> rozdzielacz
  if( ( MPP<CVP-0.3 ) )
    BVs =Nozzles[ dP ];else
    if( ( BCP<0.5 ) )
      if( ( Zamykajacy ) )
        BVs =Nozzles[ dPm ]  //1.25;else
        BVs =Nozzles[ dPd ];else
      BVs =0;
return result;
}

void TNESt3::PLC(double mass)
{
  LoadC =1+unsigned char( mass<LoadM )*( ( TareBP+( MaxBP-TareBP )*( mass-TareM )*1.0/( LoadM-TareM ) )*1.0/MaxBP-1 );
}

void TNESt3::ForceEmptiness()
{
  ValveRes.CreatePress( 0 );
  BrakeRes.CreatePress( 0 );
  Miedzypoj.CreatePress( 0 );
  CntrlRes.CreatePress( 0 );

  BrakeStatus =0;

  ValveRes.Act(  );
  BrakeRes.Act(  );
  Miedzypoj.Act(  );
  CntrlRes.Act(  );
}


void TNESt3::SetLP(double TM, double LM, double TBP)
{
  TareM =TM;
  LoadM =LM;
  TareBP =TBP;
}


void TNESt3::SetLBP(double P)
{
  LBP =P;
}



void TNESt3::SetSize(int size,  std::string params)     //ustawianie dysz (rozmiaru ZR)
{
  static double/*?*/ const dNO1l = 1.250;
  static double/*?*/ const dNT1l = 0.510;
  static double/*?*/ const dOO1l = 0.907;
  static double/*?*/ const dOT1l = 0.524;

  int i;


  if(  Pos( "ESt3", params )>0 )
   {
     Podskok =0.7;
     Przekladniki[ 1 ] =TRura.Create;
     Przekladniki[ 3 ] =TRura.Create;
   }
  else
   {
    Podskok =-1;
    Przekladniki[ 1 ] =TRapid.Create;
    if(  Pos( "-s216", params )>0 )
      ( Przekladniki[ 1 ] as TRapid ).SetRapidParams( 2, 16 );else
      ( Przekladniki[ 1 ] as TRapid ).SetRapidParams( 2, 0 );
    Przekladniki[ 3 ] =TPrzeciwposlizg.Create;
    if(  Pos( "-ED", params )>0 )
     {
      Przekladniki[ 3 ].Free(  );
      ( Przekladniki[ 1 ] as TRapid ).SetRapidParams( 2, 18 );
      Przekladniki[ 3 ] =TPrzekED.Create;
     }
   }

  if(  Pos( "AL2", params )>0 )
    Przekladniki[ 2 ] =TPrzekCiagly.Create;else
  if(  Pos( "PZZ", params )>0 )
   Przekladniki[ 2 ] =TPrzek_PZZ.Create;else
   Przekladniki[ 2 ] =TRura.Create;

  if(  ( Pos( "3d", params )+Pos( "4d", params )>0 ) ) autom =false;else autom =true;
  if(  ( Pos( "HBG300", params )>0 ) ) HBG300 =1;else HBG300 =0;

   switch( size ) {
    case 16: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =5.0;//5.0;
      Nozzles[ dOO ] =3.4;//3.4;
      Nozzles[ dTN ] =2.0;
      Nozzles[ dTO ] =1.75;
      Nozzles[ dP ] =3.8;
      Nozzles[ dPd ] =2.70;
      Nozzles[ dPm ] =1.25;
      Nozzles[ dPO ] =Nozzles[ dON ];
      Nozzles[ dPT ] =Nozzles[ dTN ];
     }
    case 14: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =4.3;
      Nozzles[ dOO ] =2.85;
      Nozzles[ dTN ] =1.83;
      Nozzles[ dTO ] =1.57;
      Nozzles[ dP ] =3.4;
      Nozzles[ dPd ] =2.20;
      Nozzles[ dPm ] =1.10;
      Nozzles[ dPO ] =Nozzles[ dON ];
      Nozzles[ dPT ] =Nozzles[ dTN ];
     }
    case 12: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =3.7;
      Nozzles[ dOO ] =2.50;
      Nozzles[ dTN ] =1.65;
      Nozzles[ dTO ] =1.39;
      Nozzles[ dP ] =2.65;
      Nozzles[ dPd ] =1.80;
      Nozzles[ dPm ] =0.85;
      Nozzles[ dPO ] =Nozzles[ dON ];
      Nozzles[ dPT ] =Nozzles[ dTN ];
     }
    case 10: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =3.1;
      Nozzles[ dOO ] =2.0;
      Nozzles[ dTN ] =1.35;
      Nozzles[ dTO ] =1.13;
      Nozzles[ dP ] =1.6;
      Nozzles[ dPd ] =1.55;
      Nozzles[ dPm ] =0.7;
      Nozzles[ dPO ] =Nozzles[ dON ];
      Nozzles[ dPT ] =Nozzles[ dTN ];
     }
    case 200: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =dNO1l;
      Nozzles[ dOO ] =dOO1l*1.0/1.15;
      Nozzles[ dTN ] =dNT1l;
      Nozzles[ dTO ] =dOT1l;
      Nozzles[ dP ] =7.4;
      Nozzles[ dPd ] =5.3;
      Nozzles[ dPm ] =2.5;
      Nozzles[ dPO ] =7.28;
      Nozzles[ dPT ] =2.96;
     }
    case 375: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =dNO1l;
      Nozzles[ dOO ] =dOO1l*1.0/1.15;
      Nozzles[ dTN ] =dNT1l;
      Nozzles[ dTO ] =dOT1l;
      Nozzles[ dP ] =13.0;
      Nozzles[ dPd ] =9.6;
      Nozzles[ dPm ] =4.4;
      Nozzles[ dPO ] =9.92;
      Nozzles[ dPT ] =3.99;
     }
    case 150: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =dNO1l;
      Nozzles[ dOO ] =dOO1l;
      Nozzles[ dTN ] =dNT1l;
      Nozzles[ dTO ] =dOT1l;
      Nozzles[ dP ] =5.8;
      Nozzles[ dPd ] =4.1;
      Nozzles[ dPm ] =1.9;
      Nozzles[ dPO ] =6.33;
      Nozzles[ dPT ] =2.58;
     }
    case 100: 
     {   //dON,dOO,dTN,dTO,dP,dS
      Nozzles[ dON ] =dNO1l;
      Nozzles[ dOO ] =dOO1l;
      Nozzles[ dTN ] =dNT1l;
      Nozzles[ dTO ] =dOT1l;
      Nozzles[ dP ] =4.2;
      Nozzles[ dPd ] =2.9;
      Nozzles[ dPm ] =1.4;
      Nozzles[ dPO ] =5.19;
      Nozzles[ dPT ] =2.14;
     }
    default:
     {
      Nozzles[ dON ] =0;
      Nozzles[ dOO ] =0;
      Nozzles[ dTN ] =0;
      Nozzles[ dTO ] =0;
      Nozzles[ dP ] =0;
      Nozzles[ dPd ] =0;
      Nozzles[ dPm ] =0;
      }
  }

  Nozzles[ dSd ] =1.1;
  Nozzles[ dSm ] =0.9;

  //przeliczanie z mm^2 na l/m
  { long i_end = dMAX+1 ; for( i = 0 ; i < i_end ; ++i )
   {
    Nozzles[ i ] =d2A( Nozzles[ i ] ); //(/1000^2*pi/4*1000)
   }}

  { long i_end = 4 ; for( i = 1 ; i < i_end ; ++i )
   {
    Przekladniki[ i ].BrakeRes =&BrakeRes;
    Przekladniki[ i ].CreateCap( i );
    Przekladniki[ i ].CreatePress( BrakeCyl.P );
    if(  i<3 )
      Przekladniki[ i ].Next =&Przekladniki[ i+1 ];else
      Przekladniki[ i ].Next =&BrakeCyl;
   }}
}



//END
