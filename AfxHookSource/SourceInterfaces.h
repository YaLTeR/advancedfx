#pragma once

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-02-09 dominik.matrixstorm.com
//
// First changes:
// 2009-09-30 dominik.matrixstorm.com

// Based on Source engine SDK:
// Copyright (c) 1996-2005, Valve Corporation, All rights reserved

// Description:
// Interface definitions for accessing the Source engine.


class IWrpCommandArgs;
class WrpConCommands;
class WrpConVarRef;


typedef char tier0_char;

typedef void (*Tier0MsgFn)( const tier0_char* pMsg, ... );
typedef void (*Tier0DMsgFn)( const tier0_char *pGroupName, int level, const tier0_char *pMsg, ... );

// debug and message fns, available after tier0.dll has been loaded:
extern Tier0MsgFn Tier0_Msg;
extern Tier0DMsgFn Tier0_DMsg;
extern Tier0MsgFn Tier0_Warning;
extern Tier0DMsgFn Tier0_DWarning;
extern Tier0MsgFn Tier0_Log;
extern Tier0DMsgFn Tier0_DLog;
extern Tier0MsgFn Tier0_Error;

extern Tier0MsgFn Tier0_ConMsg;
extern Tier0MsgFn Tier0_ConWarning;
extern Tier0MsgFn Tier0_ConLog;
extern Tier0MsgFn Tier0_ConDMsg;
extern Tier0MsgFn Tier0_ConDWarning;
extern Tier0MsgFn Tier0_ConDLog;


namespace SOURCESDK {

#ifndef NULL
#define NULL 0
#endif

#define FORCEINLINE __forceinline
#define FORCEINLINE_CVAR FORCEINLINE

#define Assert(condition)
#define CHECK_VALID( _v)

#define Q_memcpy strncpy

typedef float vec_t;
typedef unsigned __int32 uint32;

inline vec_t BitsToFloat(uint32 i)
{
	union Convertor_t
	{
		vec_t f;
		unsigned long ul;
	}tmp;
	tmp.ul = i;
	return tmp.f;
}

#define FLOAT32_NAN_BITS     (uint32)0x7FC00000	// not a number!
#define FLOAT32_NAN          BitsToFloat( FLOAT32_NAN_BITS )

#define VEC_T_NAN FLOAT32_NAN

#define FCVAR_NONE				0 
#define FCVAR_UNREGISTERED		(1<<0)
#define FCVAR_DEVELOPMENTONLY	(1<<1)
#define FCVAR_GAMEDLL			(1<<2)
#define FCVAR_CLIENTDLL			(1<<3)
#define FCVAR_HIDDEN			(1<<4)

#define CREATEINTERFACE_PROCNAME	"CreateInterface"


class CSysModule;

typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

CreateInterfaceFn Sys_GetFactory( CSysModule *pModule );



// Vector //////////////////////////////////////////////////////////////////////

class Vector				
{
public:
	// Members
	vec_t x, y, z;

	// Initialization
	void Init(vec_t ix = 0.0f, vec_t iy = 0.0f, vec_t iz = 0.0f);
	// TODO (Ilya): Should there be an init that takes a single float for consistency?

	// shortened.
};


// QAngle //////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Degree Euler QAngle pitch, yaw, roll
//-----------------------------------------------------------------------------
class QAngleByValue;

class QAngle
{
public:
	// Members
	vec_t x, y, z;

	// Construction/destruction
	QAngle(void);
	QAngle(vec_t X, vec_t Y, vec_t Z);
	//	QAngle(RadianEuler const &angles);	// evil auto type promotion!!!

	// Allow pass-by-value
	operator QAngleByValue &() { return *((QAngleByValue *)(this)); }
	operator const QAngleByValue &() const { return *((const QAngleByValue *)(this)); }

	// Initialization
	void Init(vec_t ix = 0.0f, vec_t iy = 0.0f, vec_t iz = 0.0f);
	void Random(vec_t minVal, vec_t maxVal);

	// Got any nasty NAN's?
	bool IsValid() const;
	void Invalidate();

	// array access...
	vec_t operator[](int i) const;
	vec_t& operator[](int i);

	// Base address...
	vec_t* Base();
	vec_t const* Base() const;

	// equality
	bool operator==(const QAngle& v) const;
	bool operator!=(const QAngle& v) const;

	// arithmetic operations
	QAngle&	operator+=(const QAngle &v);
	QAngle&	operator-=(const QAngle &v);
	QAngle&	operator*=(float s);
	QAngle&	operator/=(float s);

	// Get the vector's magnitude.
	vec_t	Length() const;
	vec_t	LengthSqr() const;

	// negate the QAngle components
	//void	Negate(); 

	// No assignment operators either...
	QAngle& operator=(const QAngle& src);

#ifndef VECTOR_NO_SLOW_OPERATIONS
	// copy constructors

	// arithmetic operations
	QAngle	operator-(void) const;

	QAngle	operator+(const QAngle& v) const;
	QAngle	operator-(const QAngle& v) const;
	QAngle	operator*(float fl) const;
	QAngle	operator/(float fl) const;
#else

private:
	// No copy constructors allowed if we're in optimal mode
	QAngle(const QAngle& vOther);

#endif
};

//-----------------------------------------------------------------------------
// Allows us to specifically pass the vector by value when we need to
//-----------------------------------------------------------------------------
class QAngleByValue : public QAngle
{
public:
	// Construction/destruction:
	QAngleByValue(void) : QAngle() {}
	QAngleByValue(vec_t X, vec_t Y, vec_t Z) : QAngle(X, Y, Z) {}
	QAngleByValue(const QAngleByValue& vOther) { *this = vOther; }
};


inline void VectorAdd(const QAngle& a, const QAngle& b, QAngle& result)
{
	CHECK_VALID(a);
	CHECK_VALID(b);
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
}

inline void VectorMA(const QAngle &start, float scale, const QAngle &direction, QAngle &dest)
{
	CHECK_VALID(start);
	CHECK_VALID(direction);
	dest.x = start.x + scale * direction.x;
	dest.y = start.y + scale * direction.y;
	dest.z = start.z + scale * direction.z;
}


//-----------------------------------------------------------------------------
// constructors
//-----------------------------------------------------------------------------
inline QAngle::QAngle(void)
{
#ifdef _DEBUG
#ifdef VECTOR_PARANOIA
	// Initialize to NAN to catch errors
	x = y = z = VEC_T_NAN;
#endif
#endif
}

inline QAngle::QAngle(vec_t X, vec_t Y, vec_t Z)
{
	x = X; y = Y; z = Z;
	CHECK_VALID(*this);
}


//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------
inline void QAngle::Init(vec_t ix, vec_t iy, vec_t iz)
{
	x = ix; y = iy; z = iz;
	CHECK_VALID(*this);
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------
inline QAngle& QAngle::operator=(const QAngle &vOther)
{
	CHECK_VALID(vOther);
	x = vOther.x; y = vOther.y; z = vOther.z;
	return *this;
}


//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------
inline vec_t& QAngle::operator[](int i)
{
	Assert((i >= 0) && (i < 3));
	return ((vec_t*)this)[i];
}

inline vec_t QAngle::operator[](int i) const
{
	Assert((i >= 0) && (i < 3));
	return ((vec_t*)this)[i];
}


//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
inline vec_t* QAngle::Base()
{
	return (vec_t*)this;
}

inline vec_t const* QAngle::Base() const
{
	return (vec_t const*)this;
}


//-----------------------------------------------------------------------------
// Invalidate
//-----------------------------------------------------------------------------

inline void QAngle::Invalidate()
{
	//#ifdef _DEBUG
	//#ifdef VECTOR_PARANOIA
	x = y = z = VEC_T_NAN;
	//#endif
	//#endif
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------
inline bool QAngle::operator==(const QAngle& src) const
{
	CHECK_VALID(src);
	CHECK_VALID(*this);
	return (src.x == x) && (src.y == y) && (src.z == z);
}

inline bool QAngle::operator!=(const QAngle& src) const
{
	CHECK_VALID(src);
	CHECK_VALID(*this);
	return (src.x != x) || (src.y != y) || (src.z != z);
}


//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------
inline void VectorCopy(const QAngle& src, QAngle& dst)
{
	CHECK_VALID(src);
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
}


//-----------------------------------------------------------------------------
// standard math operations
//-----------------------------------------------------------------------------
inline QAngle& QAngle::operator+=(const QAngle& v)
{
	CHECK_VALID(*this);
	CHECK_VALID(v);
	x += v.x; y += v.y; z += v.z;
	return *this;
}

inline QAngle& QAngle::operator-=(const QAngle& v)
{
	CHECK_VALID(*this);
	CHECK_VALID(v);
	x -= v.x; y -= v.y; z -= v.z;
	return *this;
}

inline QAngle& QAngle::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	z *= fl;
	CHECK_VALID(*this);
	return *this;
}

inline QAngle& QAngle::operator/=(float fl)
{
	Assert(fl != 0.0f);
	float oofl = 1.0f / fl;
	x *= oofl;
	y *= oofl;
	z *= oofl;
	CHECK_VALID(*this);
	return *this;
}


inline vec_t QAngle::LengthSqr() const
{
	CHECK_VALID(*this);
	return x * x + y * y + z * z;
}


//-----------------------------------------------------------------------------
// arithmetic operations (SLOW!!)
//-----------------------------------------------------------------------------
#ifndef VECTOR_NO_SLOW_OPERATIONS

inline QAngle QAngle::operator-(void) const
{
	QAngle ret(-x, -y, -z);
	return ret;
}

inline QAngle QAngle::operator+(const QAngle& v) const
{
	QAngle res;
	res.x = x + v.x;
	res.y = y + v.y;
	res.z = z + v.z;
	return res;
}

inline QAngle QAngle::operator-(const QAngle& v) const
{
	QAngle res;
	res.x = x - v.x;
	res.y = y - v.y;
	res.z = z - v.z;
	return res;
}

inline QAngle QAngle::operator*(float fl) const
{
	QAngle res;
	res.x = x * fl;
	res.y = y * fl;
	res.z = z * fl;
	return res;
}

inline QAngle QAngle::operator/(float fl) const
{
	QAngle res;
	res.x = x / fl;
	res.y = y / fl;
	res.z = z / fl;
	return res;
}

inline QAngle operator*(float fl, const QAngle& v)
{
	QAngle ret(v * fl);
	return ret;
}

#endif // VECTOR_NO_SLOW_OPERATIONS


struct matrix3x4_t
{
	matrix3x4_t() {}
	matrix3x4_t(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	void Init(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector &vecOrigin)
	{
		m_flMatVal[0][0] = xAxis.x; m_flMatVal[0][1] = yAxis.x; m_flMatVal[0][2] = zAxis.x; m_flMatVal[0][3] = vecOrigin.x;
		m_flMatVal[1][0] = xAxis.y; m_flMatVal[1][1] = yAxis.y; m_flMatVal[1][2] = zAxis.y; m_flMatVal[1][3] = vecOrigin.y;
		m_flMatVal[2][0] = xAxis.z; m_flMatVal[2][1] = yAxis.z; m_flMatVal[2][2] = zAxis.z; m_flMatVal[2][3] = vecOrigin.z;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	matrix3x4_t(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector &vecOrigin)
	{
		Init(xAxis, yAxis, zAxis, vecOrigin);
	}

	inline void SetOrigin(Vector const & p)
	{
		m_flMatVal[0][3] = p.x;
		m_flMatVal[1][3] = p.y;
		m_flMatVal[2][3] = p.z;
	}

	inline void Invalidate(void)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_flMatVal[i][j] = VEC_T_NAN;
			}
		}
	}

	float *operator[](int i) { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	const float *operator[](int i) const { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	float *Base() { return &m_flMatVal[0][0]; }
	const float *Base() const { return &m_flMatVal[0][0]; }

	float m_flMatVal[3][4];
};

//-----------------------------------------------------------------------------
// Quaternion
//-----------------------------------------------------------------------------

class RadianEuler;

class Quaternion				// same data-layout as engine's vec4_t,
{								//		which is a vec_t[4]
public:
	inline Quaternion(void) {

		// Initialize to NAN to catch errors
#ifdef _DEBUG
#ifdef VECTOR_PARANOIA
		x = y = z = w = VEC_T_NAN;
#endif
#endif
	}
	inline Quaternion(vec_t ix, vec_t iy, vec_t iz, vec_t iw) : x(ix), y(iy), z(iz), w(iw) { }
	inline Quaternion(RadianEuler const &angle);	// evil auto type promotion!!!

	inline void Init(vec_t ix = 0.0f, vec_t iy = 0.0f, vec_t iz = 0.0f, vec_t iw = 0.0f) { x = ix; y = iy; z = iz; w = iw; }

	bool IsValid() const;
	void Invalidate();

	bool operator==(const Quaternion &src) const;
	bool operator!=(const Quaternion &src) const;

	vec_t* Base() { return (vec_t*)this; }
	const vec_t* Base() const { return (vec_t*)this; }

	// array access...
	vec_t operator[](int i) const;
	vec_t& operator[](int i);

	vec_t x, y, z, w;
};


//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------
inline vec_t& Quaternion::operator[](int i)
{
	Assert((i >= 0) && (i < 4));
	return ((vec_t*)this)[i];
}

inline vec_t Quaternion::operator[](int i) const
{
	Assert((i >= 0) && (i < 4));
	return ((vec_t*)this)[i];
}


//-----------------------------------------------------------------------------
// Equality test
//-----------------------------------------------------------------------------
inline bool Quaternion::operator==(const Quaternion &src) const
{
	return (x == src.x) && (y == src.y) && (z == src.z) && (w == src.w);
}

inline bool Quaternion::operator!=(const Quaternion &src) const
{
	return !operator==(src);
}


// VMatrix /////////////////////////////////////////////////////////////////////

class MdtMatrix;
typedef MdtMatrix VMatrix;

class MdtMatrix
{
public:
	MdtMatrix();
	MdtMatrix(const MdtMatrix & mdtMatrix);

	// The matrix.
	vec_t		m[4][4];
};


// Rendering related

class CGlobalVarsBase;

// Command / Cvar related:

// we hack around a bit here:
typedef void (*WrpCommandCallback)(::IWrpCommandArgs * args);

class ConVar_003;

typedef void ( *FnChangeCallback_003 )( ConVar_003 *var, char const *pOldString );
typedef void ( *FnCommandCallback_003 )( void );

#define COMMAND_COMPLETION_MAXITEMS_004 64
#define COMMAND_COMPLETION_ITEM_LENGTH_004 64

class CCommand_004;
class ConVar_004;
class IConVar_004;

typedef void ( *FnChangeCallback_t_004 )( IConVar_004 *var, const char *pOldValue, float flOldValue );
typedef void ( *FnCommandCallbackV1_t_004 )( void );
typedef void ( *FnCommandCallback_t_004 )( const CCommand_004 &command );

typedef int CVarDLLIdentifier_t_004;

class ICommandCallback_004
{
public:
	virtual void CommandCallback( const CCommand_004 &command ) = 0;
};


typedef int CVarDLLIdentifier_t_007;

// IMemAlloc ///////////////////////////////////////////////////////////////////////

class IMemAlloc_csgo
{
public:
	virtual void _UNUSED_000(void) = 0;
	virtual void _UNUSED_001(void) = 0;
	virtual void _UNUSED_002(void) = 0;
	virtual void _UNUSED_003(void) = 0;
	virtual void _UNUSED_004(void) = 0;

	virtual void Free( void *pMem ) = 0; //:005

	virtual void _UNUSED_006(void) = 0;
	virtual void _UNUSED_007(void) = 0;

	virtual size_t GetSize(void *pMem) = 0; //:008

	// There is more, but we don't need it at the moment
	// [....]
};

IMemAlloc_csgo * Get_g_pMemAlloc(void);

// IConCommandBaseAccessor_003 /////////////////////////////////////////////////////

class ConCommandBase_003;

class IConCommandBaseAccessor_003
{
public:
	virtual bool RegisterConCommandBase( ConCommandBase_003 *pVar )=0;
};


// ConCommandBase_003 /////////////////////////////////////////////////////////////

/// <remarks> DO NOT CHANGE WITHOUT KNOWING WHAT YOU DO, DIRECTLY ACCESSED BY SOURCE ENGINE! </remarks>
/// <comments> I guess if Valve used a non determisitic C++ compiler they would
///		be screwed when sharing such classes among various compile units.
///		this also means we are screwed too easily when using a different compiler.
///		</comments>
class ConCommandBase_003
{
	friend ::WrpConCommands; // ugly hack, just like Valve did

public:
	ConCommandBase_003( void );
	ConCommandBase_003( char const *pName, char const *pHelpString = 0, int flags = 0 );

	virtual						~ConCommandBase_003( void );

	virtual	bool				IsCommand( void ) const;

	// Check flag
	virtual bool				IsBitSet( int flag ) const;
	// Set flag
	virtual void				AddFlags( int flags );

	// Return name of cvar
	virtual char const			*GetName( void ) const;

	// Return help text for cvar
	virtual char const			*GetHelpText( void ) const;

	// Deal with next pointer
	const ConCommandBase_003		*GetNext( void ) const;
	void						SetNext( ConCommandBase_003 *next );
	
	virtual bool				IsRegistered( void ) const;

	// Global methods
	static void _NOT_IMPLEMENTED_GetCommands( void );
	static void	_NOT_IMPLEMENTED_AddToList( void );
	static void	_NOT_IMPLEMENTED_RemoveFlaggedCommands( void );
	static void	_NOT_IMPLEMENTED_RevertFlaggedCvars( void );
	static void _NOT_IMPLEMENTED_FindCommand( void );

protected:
	virtual void Create(char const *pName, char const *pHelpString = 0, int flags = 0 );

	virtual void Init();

	void _NOT_IMPLEMENTED_CopyString( void );

	ConCommandBase_003 * m_Next;

	static ConCommandBase_003		*s_pConCommandBases;
	static IConCommandBaseAccessor_003	*s_pAccessor;

private:
	bool m_IsRegistered;
	char * m_Name;
	char * m_HelpText;
	int m_Flags;

};


// ConCommand_003 //////////////////////////////////////////////////////////////

/// <remarks> DO NOT CHANGE WITHOUT KNOWING WHAT YOU DO, DIRECTLY ACCESSED BY SOURCE ENGINE! </remarks>
/// <comments> I guess if Valve used a non determisitic C++ compiler they would
///		be screwed when sharing such classes among various compile units.
///		this also means we are screwed too easily when using a different compiler.
///		</comments>
class ConCommand_003 : public ConCommandBase_003
{
public:
	typedef ConCommandBase_003 BaseClass;

	ConCommand_003( void );

	/// <remarks> tweaked since we don't support completition and use a callback wrapper </remarks>
	ConCommand_003( char const *pName, WrpCommandCallback callback, char const *pHelpString = 0, int flags = 0);

	virtual						~ConCommand_003( void );

	virtual	bool				IsCommand( void ) const;

	/// <remarks> we don't support autocompletition, thus we always return 0 </remarks>
	virtual int					AutoCompleteSuggest(void * dummy1, void * dummy2);

	/// <remarks> we don't support autocompletition, thus we always return false </remarks>
	virtual bool				CanAutoComplete( void );

	// Invoke the function
	virtual void				Dispatch( void );

private:
	WrpCommandCallback			m_Callback;

	virtual void				Create( char const *pName, WrpCommandCallback callback, char const *pHelpString = 0, int flags = 0);

};


// IConCommandBaseAccessor_004 /////////////////////////////////////////////////

class ConCommandBase_004;

class IConCommandBaseAccessor_004
{
public:
	virtual bool RegisterConCommandBase( ConCommandBase_004 *pVar ) = 0;
};

// ConCommandBase_004 //////////////////////////////////////////////////////////

/// <remarks> DO NOT CHANGE WITHOUT KNOWING WHAT YOU DO, DIRECTLY ACCESSED BY SOURCE ENGINE! </remarks>
/// <comments> I guess if Valve used a non determisitic C++ compiler they would
///		be screwed when sharing such classes among various compile units.
///		this also means we are screwed too easily when using a different compiler.
///		</comments>
class ConCommandBase_004
{
	friend ::WrpConCommands; // ugly hack, just like Valve did

public:
	ConCommandBase_004( void );
	ConCommandBase_004( const char *pName, const char *pHelpString = 0, int flags = 0 );

	virtual						~ConCommandBase_004( void );

	virtual	bool				IsCommand( void ) const;

	// Check flag
	virtual bool				IsFlagSet( int flag ) const;
	// Set flag
	virtual void				AddFlags( int flags );

	// Return name of cvar
	virtual const char			*GetName( void ) const;

	// Return help text for cvar
	virtual const char			*GetHelpText( void ) const;

	// Deal with next pointer
	const ConCommandBase_004		*GetNext( void ) const;
	ConCommandBase_004				*GetNext( void );
	
	virtual bool				IsRegistered( void ) const;

	// Returns the DLL identifier
	virtual CVarDLLIdentifier_t_004	GetDLLIdentifier() const;

protected:
	virtual void Create(const char *pName, const char *pHelpString = 0, int flags = 0);

	// Used internally by OneTimeInit to initialize/shutdown
	virtual void Init();
	void _NOT_IMPLEMENTED_Shutdown();

	void _NOT_IMPLEMENTED_CopyString( void );

	static ConCommandBase_004 *s_pConCommandBases;
	static IConCommandBaseAccessor_004 *s_pAccessor;

private:
	ConCommandBase_004 * m_Next;
	bool m_IsRegistered;
	char * m_Name;
	char * m_HelpText;
	int m_Flags;
};

// ConCommand_004 //////////////////////////////////////////////////////////////

class ConCommand_004 : public ConCommandBase_004
{
public:
	typedef ConCommandBase_004 BaseClass;

	/// <remarks> tweaked since we don't support completition </remarks>
	ConCommand_004(const char *pName, FnCommandCallbackV1_t_004 callback,  const char *pHelpString = 0, int flags = 0);

	/// <remarks> tweaked since we don't support completition and use a callback wrapper </remarks>
	ConCommand_004(const char *pName, WrpCommandCallback callback, const char *pHelpString = 0, int flags = 0);

	/// <remarks> tweaked since we don't support completition </remarks>
	ConCommand_004(const char *pName, ICommandCallback_004 *pCallback, const char *pHelpString = 0, int flags = 0);

	virtual ~ConCommand_004(void);

	virtual	bool IsCommand(void) const;

	/// <remarks> we don't support autocompletition, thus we always return 0 </remarks>
	virtual int AutoCompleteSuggest(void * dummy1, void * dummy2);

	/// <remarks> we don't support autocompletition, thus we always return false </remarks>
	virtual bool CanAutoComplete( void );

	virtual void Dispatch( const CCommand_004 &command );

private:
	union
	{
		FnCommandCallbackV1_t_004 m_fnCommandCallbackV1;
		WrpCommandCallback m_fnCommandCallback;
		ICommandCallback_004 *m_pCommandCallback; 
	};

	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;
};


/// <comments> This is really not my fault, this is the way Valve did it!
///		If you ever wondered who passes s.th. depeding on compiler
///		optimization etc. among DLLs and different compile units
///		- well the Source SDK does - and this time not even
///		anything virtual here - hahahahahha just gr8.
///		</comments>
///	<remarks> Do not implement this or s.th. we just use this to access
///		the class memory in the source engine. </remarks>
class CCommand_004 abstract
{
public:
	/// <remarks> NOT_IMPLEMENTED </remarks>
	CCommand_004(void);

	/// <remarks> NOT_IMPLEMENTED </remarks>
	CCommand_004(void *, void *);

	void _NOT_IMPLEMENTED_Tokenize(void);
	void _NOT_IMPLEMENTED_Reset(void);

	int ArgC() const;
	const char **ArgV() const;
	const char *ArgS() const;					// All args that occur after the 0th arg, in string form
	const char *GetCommandString() const;		// The entire command in string form, including the 0th arg
	const char *operator[]( int nIndex ) const;	// Gets at arguments
	const char *Arg( int nIndex ) const;		// Gets at arguments
	
	void _NOT_IMPLEMENTED_FindArg(void) const;
	void _NOT_IMPLEMENTED_FindArgInt(void) const;

	static int MaxCommandLength();
	static void _NOT_IMPLEMENTED_DefaultBreakSet(void);

private:
	enum
	{
		COMMAND_MAX_ARGC = 64,
		COMMAND_MAX_LENGTH = 512,
	};

	int		m_nArgc;
	int		m_nArgv0Size;
	char	m_pArgSBuffer[ COMMAND_MAX_LENGTH ];
	char	m_pArgvBuffer[ COMMAND_MAX_LENGTH ];
	const char*	m_ppArgv[ COMMAND_MAX_ARGC ];
};

inline int CCommand_004::MaxCommandLength()
{
	return COMMAND_MAX_LENGTH - 1;
}

inline int CCommand_004::ArgC() const
{
	return m_nArgc;
}

inline const char **CCommand_004::ArgV() const
{
	return m_nArgc ? (const char**)m_ppArgv : 0;
}

inline const char *CCommand_004::ArgS() const
{
	return m_nArgv0Size ? &m_pArgSBuffer[m_nArgv0Size] : "";
}

inline const char *CCommand_004::GetCommandString() const
{
	return m_nArgc ? m_pArgSBuffer : "";
}

inline const char *CCommand_004::Arg( int nIndex ) const
{
	// FIXME: Many command handlers appear to not be particularly careful
	// about checking for valid argc range. For now, we're going to
	// do the extra check and return an empty string if it's out of range
	if ( nIndex < 0 || nIndex >= m_nArgc )
		return "";
	return m_ppArgv[nIndex];
}

inline const char *CCommand_004::operator[]( int nIndex ) const
{
	return Arg( nIndex );
}


// IAppSystem //////////////////////////////////////////////////////////////////

enum InitReturnVal_t
{
	INIT_FAILED = 0,
	INIT_OK,

	INIT_LAST_VAL,
};


class IAppSystem abstract
{
public:
	// Here's where the app systems get to learn about each other 
	virtual bool Connect( CreateInterfaceFn factory ) = 0;
	virtual void Disconnect() = 0;

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void *QueryInterface( const char *pInterfaceName ) = 0;

	// Init, shutdown
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;
};

// IAppSystem_csgo ////////////////////////////////////////////////////////////

struct AppSystemInfo_t
{
	const char *m_pModuleName;
	const char *m_pInterfaceName;
};

enum AppSystemTier_t
{
	APP_SYSTEM_TIER0 = 0,
	APP_SYSTEM_TIER1,
	APP_SYSTEM_TIER2,
	APP_SYSTEM_TIER3,

	APP_SYSTEM_TIER_OTHER,
};

class IAppSystem_csgo abstract
{
public:
	// Here's where the app systems get to learn about each other 
	virtual bool Connect( CreateInterfaceFn factory ) = 0;
	virtual void Disconnect() = 0;

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void *QueryInterface( const char *pInterfaceName ) = 0;

	// Init, shutdown
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;

	// Returns all dependent libraries
	virtual const AppSystemInfo_t* GetDependencies() = 0;

	// Returns the tier
	virtual AppSystemTier_t GetTier() = 0;

	// Reconnect to a particular interface
	virtual void Reconnect( CreateInterfaceFn factory, const char *pInterfaceName ) = 0;

	// Returns whether or not the app system is a singleton
	virtual bool IsSingleton() = 0;
};



// ICvar_003 ///////////////////////////////////////////////////////////////////

#define VENGINE_CVAR_INTERFACE_VERSION_003 "VEngineCvar003"

/// <comments>
///		Supported by: Counter-Strike Source
///		</comments>
class ICvar_003 abstract : public IAppSystem
{
public:
	// Try to register cvar
	virtual void			RegisterConCommandBase ( ConCommandBase_003 *variable ) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual char const*		GetCommandLineValue( char const *pVariableName ) = 0;

	// Try to find the cvar pointer by name
	virtual ConVar_003			*FindVar ( const char *var_name ) = 0;
	virtual const ConVar_003	*FindVar ( const char *var_name ) const = 0;

	// Get first ConCommandBase to allow iteration
	virtual ConCommandBase_003	*GetCommands( void ) = 0;

	// Removes all cvars with flag bit set
	virtual void			UnlinkVariables( int flag ) = 0;

	// Install a global change callback (to be called when any convar changes) 
	virtual void			InstallGlobalChangeCallback( FnChangeCallback_003 callback ) = 0;
	virtual void			CallGlobalChangeCallback( ConVar_003 *var, char const *pOldString ) = 0;
};

// ICvar_004 ///////////////////////////////////////////////////////////////////

#define VENGINE_CVAR_INTERFACE_VERSION_004 "VEngineCvar004"

/// <comments>
///		Supported by: Portal First Slice
///		</comments>
class ICvar_004 abstract : public IAppSystem
{
public:
/*	virtual void _Unknown_001(void);
	virtual void _Unknown_002(void);
	virtual void _Unknown_003(void);
*/
	// Allocate a unique DLL identifier
	virtual CVarDLLIdentifier_t_004 AllocateDLLIdentifier() = 0;

	// Register, unregister commands
	virtual void			RegisterConCommand( ConCommandBase_004 *pCommandBase ) = 0;
	virtual void			UnregisterConCommand( ConCommandBase_004 *pCommandBase ) = 0;
	virtual void			UnregisterConCommands( CVarDLLIdentifier_t_004 id ) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual const char*		GetCommandLineValue( const char *pVariableName ) = 0;

	// Try to find the cvar pointer by name
	virtual ConCommandBase_004 *FindCommandBase( const char *name ) = 0;
	virtual const ConCommandBase_004 *FindCommandBase( const char *name ) const = 0;
	virtual ConVar_004			*FindVar ( const char *var_name ) = 0;
	virtual const ConVar_004	*FindVar ( const char *var_name ) const = 0;
	virtual ConCommand_004		*FindCommand( const char *name ) = 0;
	virtual const ConCommand_004 *FindCommand( const char *name ) const = 0;

	// Get first ConCommandBase to allow iteration
	virtual ConCommandBase_004	*GetCommands( void ) = 0;
	virtual const ConCommandBase_004 *GetCommands( void ) const = 0;

	// Install a global change callback (to be called when any convar changes) 
	virtual void			InstallGlobalChangeCallback( FnChangeCallback_t_004 callback ) = 0;
	virtual void			RemoveGlobalChangeCallback( FnChangeCallback_t_004 callback ) = 0;
	virtual void			CallGlobalChangeCallbacks( ConVar_004 *var, const char *pOldString, float flOldValue ) = 0;

	virtual void _UNUSED_InstallConsoleDisplayFunc(void)=0;
	virtual void _UNUSED_RemoveConsoleDisplayFunc(void)=0;
	virtual void _UNUSED_ConsoleColorPrintf(void)=0;
	virtual void _UNUSED_ConsolePrintf(void)=0;
	virtual void _UNUSED_ConsoleDPrintf(void)=0;
	virtual void _UNUSED_RevertFlaggedConVars(void)=0;
	virtual void _UNUSED_InstallCVarQuery(void)=0;

#if defined( _X360 )
	virtual void _UNUSED_PublishToVXConsole(void)=0;
#endif
};

// ICvar_007 ///////////////////////////////////////////////////////////////////

#define CVAR_INTERFACE_VERSION_007 "VEngineCvar007"

class ConCommandBase_007;
class ConVar_007;
class ConCommand_007;
class IConVar_007;
class CCommand_007;

typedef void ( *FnChangeCallback_t_007 )( IConVar_007 *var, const char *pOldValue, float flOldValue );

class IConCommandBaseAccessor_007
{
public:
	// Flags is a combination of FCVAR flags in cvar.h.
	// hOut is filled in with a handle to the variable.
	virtual bool RegisterConCommandBase( ConCommandBase_007 *pVar ) = 0;
};

typedef void ( *FnCommandCallbackV1_t_007 )( void );
typedef void ( *FnCommandCallback_t_007 )( const CCommand_007 &command );

#define COMMAND_COMPLETION_MAXITEMS_007		64
#define COMMAND_COMPLETION_ITEM_LENGTH_007	64

//-----------------------------------------------------------------------------
// Returns 0 to COMMAND_COMPLETION_MAXITEMS worth of completion strings
//-----------------------------------------------------------------------------
typedef int  ( *FnCommandCompletionCallback_007 )( const char *partial, char commands[ COMMAND_COMPLETION_MAXITEMS_007	 ][ COMMAND_COMPLETION_ITEM_LENGTH_007 ] );

class ICommandCallback_007
{
public:
	virtual void CommandCallback( const CCommand_007 &command ) = 0;
};

class ICommandCompletionCallback_007;


class ConCommandBase_007
{
	friend ::WrpConCommands; // ugly hack, just like Valve did

public:
								ConCommandBase_007( void );
								ConCommandBase_007( const char *pName, const char *pHelpString = 0, 
									int flags = 0 );

	virtual						~ConCommandBase_007( void );

	virtual	bool				IsCommand( void ) const;

	// Check flag
	virtual bool				IsFlagSet( int flag ) const;
	// Set flag
	virtual void				AddFlags( int flags );
	// Clear flag
	virtual void				RemoveFlags( int flags );

	virtual int					GetFlags() const;

	// Return name of cvar
	virtual const char			*GetName( void ) const;

	// Return help text for cvar
	virtual const char			*GetHelpText( void ) const;

	// Deal with next pointer
	const ConCommandBase_007		*GetNext( void ) const;
	ConCommandBase_007				*GetNext( void );
	
	virtual bool				IsRegistered( void ) const;

	// Returns the DLL identifier
	virtual CVarDLLIdentifier_t_007	GetDLLIdentifier() const;

protected:
	virtual void				Create( const char *pName, const char *pHelpString = 0, 
									int flags = 0 );

	// Used internally by OneTimeInit to initialize/shutdown
	virtual void				Init();
	void _NOT_IMPLEMENTED_Shutdown();

	// Internal copy routine ( uses new operator from correct module )
	void _NOT_IMPLEMENTED_CopyString(void);

private:
	// Next ConVar in chain
	// Prior to register, it points to the next convar in the DLL.
	// Once registered, though, m_pNext is reset to point to the next
	// convar in the global list
	ConCommandBase_007				*m_pNext;

	// Has the cvar been added to the global list?
	bool						m_bRegistered;

	// Static data
	char 					*m_pszName;
	char 					*m_pszHelpString;
	
	// ConVar flags
	int							m_nFlags;

protected:
	// ConVars add themselves to this list for the executable. 
	// Then ConVar_Register runs through  all the console variables 
	// and registers them into a global list stored in vstdlib.dll
	static ConCommandBase_007		*s_pConCommandBases;

	// ConVars in this executable use this 'global' to access values.
	static IConCommandBaseAccessor_007	*s_pAccessor;
};


//-----------------------------------------------------------------------------
// Command tokenizer
//-----------------------------------------------------------------------------
class CCommand_007
{
public:
	CCommand_007();
	CCommand_007( int nArgC, const char **ppArgV );
	bool _NOT_IMPLEMENTED_Tokenize(void);
	void _NOT_IMPLEMENTED_Reset(void);

	int ArgC() const;
	const char **ArgV() const;
	const char *ArgS() const;					// All args that occur after the 0th arg, in string form
	const char *GetCommandString() const;		// The entire command in string form, including the 0th arg
	const char *operator[]( int nIndex ) const;	// Gets at arguments
	const char *Arg( int nIndex ) const;		// Gets at arguments
	
	// Helper functions to parse arguments to commands.
	void _NOT_IMPLEMENTED_FindArg(void) const;
	void _NOT_IMPLEMENTED_FindArgInt(void) const;

	static int MaxCommandLength();
	static void _NOT_IMPLEMENTED_DefaultBreakSet();

private:
	enum
	{
		COMMAND_MAX_ARGC = 64,
		COMMAND_MAX_LENGTH = 512,
	};

	int		m_nArgc;
	int		m_nArgv0Size;
	char	m_pArgSBuffer[ COMMAND_MAX_LENGTH ];
	char	m_pArgvBuffer[ COMMAND_MAX_LENGTH ];
	const char*	m_ppArgv[ COMMAND_MAX_ARGC ];
};

inline int CCommand_007::MaxCommandLength()
{
	return COMMAND_MAX_LENGTH - 1;
}

inline int CCommand_007::ArgC() const
{
	return m_nArgc;
}

inline const char **CCommand_007::ArgV() const
{
	return m_nArgc ? (const char**)m_ppArgv : 0;
}

inline const char *CCommand_007::ArgS() const
{
	return m_nArgv0Size ? &m_pArgSBuffer[m_nArgv0Size] : "";
}

inline const char *CCommand_007::GetCommandString() const
{
	return m_nArgc ? m_pArgSBuffer : "";
}

inline const char *CCommand_007::Arg( int nIndex ) const
{
	// FIXME: Many command handlers appear to not be particularly careful
	// about checking for valid argc range. For now, we're going to
	// do the extra check and return an empty string if it's out of range
	if ( nIndex < 0 || nIndex >= m_nArgc )
		return "";
	return m_ppArgv[nIndex];
}

inline const char *CCommand_007::operator[]( int nIndex ) const
{
	return Arg( nIndex );
}


//-----------------------------------------------------------------------------
// Purpose: The console invoked command
//-----------------------------------------------------------------------------
class ConCommand_007 : public ConCommandBase_007
{
friend class CCvar_007;

public:
	typedef ConCommandBase_007 BaseClass;

	/// <remarks>not implemented</remarks>
	ConCommand_007( const char *pName, FnCommandCallbackV1_t_007 callback, 
		const char *pHelpString = 0, int flags = 0, FnCommandCompletionCallback_007 completionFunc = 0 );
	
	/// <remarks> tweaked since we don't support completition and use a callback wrapper </remarks>
	ConCommand_007( const char *pName, WrpCommandCallback callback, 
		const char *pHelpString = 0, int flags = 0, FnCommandCompletionCallback_007 completionFunc = 0 );
	
	/// <remarks>not implemented</remarks>
	ConCommand_007( const char *pName, ICommandCallback_007 *pCallback, 
		const char *pHelpString = 0, int flags = 0, ICommandCompletionCallback_007 *pCommandCompletionCallback = 0 );

	virtual ~ConCommand_007( void );

	virtual	bool IsCommand( void ) const;

	/// <remarks> we don't support autocompletition, thus we always return 0 </remarks>
	virtual int	AutoCompleteSuggest(void * dummy1, void * dummy2);

	/// <remarks> we don't support autocompletition, thus we always return false </remarks>
	virtual bool CanAutoComplete( void );

	// Invoke the function
	virtual void Dispatch( const CCommand_007 &command );

private:
	// NOTE: To maintain backward compat, we have to be very careful:
	// All public virtual methods must appear in the same order always
	// since engine code will be calling into this code, which *does not match*
	// in the mod code; it's using slightly different, but compatible versions
	// of this class. Also: Be very careful about adding new fields to this class.
	// Those fields will not exist in the version of this class that is instanced
	// in mod code.

	// Call this function when executing the command
	union
	{
		FnCommandCallbackV1_t_007 m_fnCommandCallbackV1;
		WrpCommandCallback m_fnCommandCallback;
		ICommandCallback_007 *m_pCommandCallback; 
	};

	union
	{
		FnCommandCompletionCallback_007	m_fnCompletionCallback;
		ICommandCompletionCallback_007 *m_pCommandCompletionCallback;
	};

	bool m_bHasCompletionCallback : 1;
	bool m_bUsingNewCommandCallback : 1;
	bool m_bUsingCommandCallbackInterface : 1;
};

class Color;

class IConsoleDisplayFunc abstract
{
public:
	virtual void ColorPrint( const Color& clr, const char *pMessage ) = 0;
	virtual void Print( const char *pMessage ) = 0;
	virtual void DPrint( const char *pMessage ) = 0;

	virtual void GetConsoleText( char *pchText, size_t bufSize ) const = 0;
};


class ICvar_007 abstract : public IAppSystem_csgo
{
public:
	// Allocate a unique DLL identifier
	virtual CVarDLLIdentifier_t_007 AllocateDLLIdentifier() = 0;

	// Register, unregister commands
	virtual void			RegisterConCommand( ConCommandBase_007 *pCommandBase ) = 0;
	virtual void			UnregisterConCommand( ConCommandBase_007 *pCommandBase ) = 0;
	virtual void			UnregisterConCommands( CVarDLLIdentifier_t_007 id ) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual const char*		GetCommandLineValue( const char *pVariableName ) = 0;

	// Try to find the cvar pointer by name
	virtual ConCommandBase_007 *FindCommandBase( const char *name ) = 0;
	virtual const ConCommandBase_007 *FindCommandBase( const char *name ) const = 0;
	virtual ConVar_007			*FindVar ( const char *var_name ) = 0;
	virtual const ConVar_007	*FindVar ( const char *var_name ) const = 0;
	virtual ConCommand_007		*FindCommand( const char *name ) = 0;
	virtual const ConCommand_007 *FindCommand( const char *name ) const = 0;



	// Install a global change callback (to be called when any convar changes) 
	virtual void			InstallGlobalChangeCallback( FnChangeCallback_t_007 callback ) = 0;
	virtual void			RemoveGlobalChangeCallback( FnChangeCallback_t_007 callback ) = 0;
	virtual void			CallGlobalChangeCallbacks( ConVar_007 *var, const char *pOldString, float flOldValue ) = 0;

	virtual void			InstallConsoleDisplayFunc( IConsoleDisplayFunc* pDisplayFunc ) = 0;
	virtual void			RemoveConsoleDisplayFunc( IConsoleDisplayFunc* pDisplayFunc ) = 0;
	virtual void			_UNUSED_ConsoleColorPrintf( void ) const = 0;
	virtual void			_UNUSED_ConsolePrintf( void ) const = 0;
	virtual void			_UNUSED_ConsoleDPrintf( void ) const = 0;
	virtual void			_UNUSED_RevertFlaggedConVars( void ) = 0;
	virtual void			_UNUSED_InstallCVarQuery( void ) = 0;

#if defined( _X360 )
	virtual void			_UNUSED_PublishToVXConsole( ) = 0;
#endif

	virtual void			_UNUSED_SetMaxSplitScreenSlots( int nSlots ) = 0;
	virtual int				_UNUSED_GetMaxSplitScreenSlots() const = 0;

	virtual void			_UNUSED_AddSplitScreenConVars() = 0;
	virtual void			_UNUSED_RemoveSplitScreenConVars( CVarDLLIdentifier_t_007 id ) = 0;

	virtual int				_UNUSED_GetConsoleDisplayFuncCount() const = 0;
	virtual void			_UNUSED_GetConsoleText( int nDisplayFuncIndex, char *pchText, size_t bufSize ) const = 0;

	virtual bool			_UNUSED_IsMaterialThreadSetAllowed( ) const = 0;
	virtual void			_UNUSED_QueueMaterialThreadSetValue( ConVar_007 *pConVar, const char *pValue ) = 0;
	virtual void			_UNUSED_QueueMaterialThreadSetValue( ConVar_007 *pConVar, int nValue ) = 0;
	virtual void			_UNUSED_QueueMaterialThreadSetValue( ConVar_007 *pConVar, float flValue ) = 0;
	virtual bool			_UNUSED_HasQueuedMaterialThreadConVarSets() const = 0;
	virtual int				_UNUSED_ProcessQueuedMaterialThreadConVarSets() = 0;

protected:	class ICVarIteratorInternal;
public:
	/// Iteration over all cvars. 
	/// (THIS IS A SLOW OPERATION AND YOU SHOULD AVOID IT.)
	/// usage: 
	/// { ICVar::Iterator iter(g_pCVar); 
	///   for ( iter.SetFirst() ; iter.IsValid() ; iter.Next() )
	///   {  
	///       ConCommandBase *cmd = iter.Get();
	///   } 
	/// }
	/// The Iterator class actually wraps the internal factory methods
	/// so you don't need to worry about new/delete -- scope takes care
	//  of it.
	/// We need an iterator like this because we can't simply return a 
	/// pointer to the internal data type that contains the cvars -- 
	/// it's a custom, protected class with unusual semantics and is
	/// prone to change.
	class Iterator
	{
	public:
		inline Iterator(ICvar_007 *icvar);
		inline ~Iterator(void);
		inline void		SetFirst( void );
		inline void		Next( void );
		inline bool		IsValid( void );
		inline ConCommandBase_007 *Get( void );
	private:
		ICVarIteratorInternal *m_pIter;
	};

protected:
	// internals for  ICVarIterator
	class ICVarIteratorInternal
	{
	public:
		virtual void		SetFirst( void ) = 0;
		virtual void		Next( void ) = 0;
		virtual	bool		IsValid( void ) = 0;
		virtual ConCommandBase_007 *Get( void ) = 0;
	};

	virtual ICVarIteratorInternal	*FactoryInternalIterator( void ) = 0;
	friend class Iterator;
};

inline ICvar_007::Iterator::Iterator(ICvar_007 *icvar)
{
	m_pIter = icvar->FactoryInternalIterator();
}

inline ICvar_007::Iterator::~Iterator( void )
{
	Get_g_pMemAlloc()->Free(m_pIter);
}

inline void ICvar_007::Iterator::SetFirst( void )
{
	m_pIter->SetFirst();
}

inline void ICvar_007::Iterator::Next( void )
{
	m_pIter->Next();
}

inline bool ICvar_007::Iterator::IsValid( void )
{
	return m_pIter->IsValid();
}

inline ConCommandBase_007 * ICvar_007::Iterator::Get( void )
{
	return m_pIter->Get();
}

//-----------------------------------------------------------------------------
// Abstract interface for ConVars
//-----------------------------------------------------------------------------
class IConVar_007 abstract
{
public:
	// Value set
	virtual void SetValue( const char *pValue ) = 0;
	virtual void SetValue( float flValue ) = 0;
	virtual void SetValue( int nValue ) = 0;
	virtual void SetValue( Color value ) = 0;

	// Return name of command
	virtual const char *GetName( void ) const = 0;

	// Return name of command (usually == GetName(), except in case of FCVAR_SS_ADDED vars
	virtual const char *GetBaseName( void ) const = 0;

	// Accessors.. not as efficient as using GetState()/GetInfo()
	// if you call these methods multiple times on the same IConVar
	virtual bool IsFlagSet( int nFlag ) const = 0;

	virtual int GetSplitScreenPlayerSlot() const = 0;
};

template< class T, class I = int >
/// <remarks>Warning, only required elements declared and defined!</remarks>
class CUtlMemory_007
{
public:
	//
	// We don't need this
};

template< class T, class A = CUtlMemory_007<T> >
/// <remarks>Warning, only required elements declared and defined!</remarks>
class CUtlVector_007
{
	typedef A CAllocator;
public:
	typedef T ElemType_t;

protected:
	CAllocator m_Memory;
	int m_Size;

#ifndef _X360
	// For easier access to the elements through the debugger
	// it's in release builds so this can be used in libraries correctly
	T *m_pElements;

#else
#endif
};

//-----------------------------------------------------------------------------
// Called when a ConVar changes value
// NOTE: For FCVAR_NEVER_AS_STRING ConVars, pOldValue == NULL
//-----------------------------------------------------------------------------
typedef void ( *FnChangeCallback_t_007 )( IConVar_007 *var, const char *pOldValue, float flOldValue );

//-----------------------------------------------------------------------------
// Purpose: A console variable
//-----------------------------------------------------------------------------
/// <remarks>Warning, only required elements declared and defined!</remarks>
class ConVar_007 abstract : public ConCommandBase_007, public IConVar_007
{
friend class CCvar_007;
friend class ConVarRef_007;
friend class SplitScreenConVarRef_007;

friend class ::WrpConVarRef;

public:
	typedef ConCommandBase_007 BaseClass;

	/// <remarks>not implemented</remarks>
	virtual						~ConVar_007( void ) = 0;

	/// <remarks>not implemented</remarks>
	virtual bool				IsFlagSet( int flag ) const = 0;

	/// <remarks>not implemented</remarks>
	virtual const char*			GetHelpText( void ) const = 0;

	/// <remarks>not implemented</remarks>
	virtual bool				IsRegistered( void ) const = 0;

	/// <remarks>not implemented</remarks>
	virtual const char			*GetName( void ) const = 0;

	// Return name of command (usually == GetName(), except in case of FCVAR_SS_ADDED vars
	/// <remarks>not implemented</remarks>
	virtual const char			*GetBaseName( void ) const = 0;

	/// <remarks>not implemented</remarks>
	//virtual int					GetSplitScreenPlayerSlot() const = 0;

	/// <remarks>not implemented</remarks>
	virtual void				AddFlags( int flags ) = 0;

	/// <remarks>not implemented</remarks>
	virtual int					GetFlags() const = 0;

	/// <remarks>not implemented</remarks>
	virtual	bool				IsCommand( void ) const = 0;

	virtual float			GetFloat(void) const = 0; // new
	virtual int			GetInt(void) const = 0; // new

	// These just call into the IConCommandBaseAccessor to check flags and set the var (which ends up calling InternalSetValue).
	virtual void				SetValue( const char *value ) = 0;
	virtual void				SetValue( float value ) = 0;
	virtual void				SetValue( int value ) = 0;
	virtual void				SetValue( Color value ) = 0;
	
	// Value
	struct CVValue_t
	{
		char						*m_pszString;
		int							m_StringLength;

		// Values
		float						m_fValue;
		int							m_nValue;
	};

	FORCEINLINE_CVAR CVValue_t &GetRawValue()
	{
		return m_Value;
	}
	FORCEINLINE_CVAR const CVValue_t &GetRawValue() const
	{
		return m_Value;
	}

private:
	// Called by CCvar when the value of a var is changing.
	//virtual void				InternalSetValue(const char *value) = 0;

	// For CVARs marked FCVAR_NEVER_AS_STRING
	//virtual void				InternalSetFloatValue( float fNewValue ) = 0;
	//virtual void				InternalSetIntValue( int nValue ) = 0;
	//virtual void				InternalSetColorValue( Color value ) = 0;

	virtual bool				ClampValue( float& value ) = 0;
	virtual void				ChangeStringValue( const char *tempVal, float flOldValue ) = 0;

	virtual void				Create( const char *pName, const char *pDefaultValue, int flags = 0,
									const char *pHelpString = 0, bool bMin = false, float fMin = 0.0,
									bool bMax = false, float fMax = false, FnChangeCallback_t_007 callback = 0 ) = 0;

	// Used internally by OneTimeInit to initialize.
	virtual void				Init() = 0;



protected:

	// This either points to "this" or it points to the original declaration of a ConVar.
	// This allows ConVars to exist in separate modules, and they all use the first one to be declared.
	// m_pParent->m_pParent must equal m_pParent (ie: m_pParent must be the root, or original, ConVar).
	ConVar_007						*m_pParent;

	// Static data
	const char					*m_pszDefaultValue;
	
	CVValue_t					m_Value;

	// Min/Max values
	bool						m_bHasMin;
	float						m_fMinVal;
	bool						m_bHasMax;
	float						m_fMaxVal;
	
	// Call this function when ConVar changes
	CUtlVector_007<FnChangeCallback_t_007> m_fnChangeCallbacks;
};

// IVEngineClient_012 //////////////////////////////////////////////////////////

#define VENGINE_CLIENT_INTERFACE_VERSION_012		"VEngineClient012"

/// <comments>
///		Supported by: Counter-Strike Source
///		</comments>
class IVEngineClient_012 abstract
{
public:
	virtual void _UNUSED_GetIntersectingSurfaces(void)=0;
	virtual void _UNUSED_GetLightForPoint(void)=0;
	virtual void _UNUSED_TraceLineMaterialAndLighting(void)=0;
	virtual void _UNUSED_ParseFile(void)=0;
	virtual void _UNUSED_CopyFile(void)=0;

	// Gets the dimensions of the game window
	virtual void				GetScreenSize( int& width, int& height ) = 0;

	// Forwards szCmdString to the server, sent reliably if bReliable is set
	virtual void				ServerCmd( const char *szCmdString, bool bReliable = true ) = 0;
	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	virtual void				ClientCmd( const char *szCmdString ) = 0;

	virtual void _UNUSED_GetPlayerInfo(void)=0;
	virtual void _UNUSED_GetPlayerForUserID(void)=0;
	virtual void _UNUSED_TextMessageGet(void)=0;

	// Returns true if the console is visible
	virtual bool				Con_IsVisible( void ) = 0;

	virtual void _UNUSED_GetLocalPlayer(void)=0;
	virtual void _UNUSED_LoadModel(void)=0;

	// Get accurate, sub-frame clock ( profiling use )
	virtual float				Time( void ) = 0; 

	// Get the exact server timesstamp ( server time ) from the last message received from the server
	virtual float				GetLastTimeStamp( void ) = 0; 

	virtual void _UNUSED_GetSentence(void)=0;
	virtual void _UNUSED_GetSentenceLength(void)=0;
	virtual void _UNUSED_IsStreaming(void)=0;

	// Copy current view orientation into va
	virtual void				GetViewAngles( QAngle& va ) = 0;
	// Set current view orientation from va
	virtual void				SetViewAngles( QAngle& va ) = 0;
	
	// Retrieve the current game's maxclients setting
	virtual int					GetMaxClients( void ) = 0;

	virtual void _UNUSED_Key_Event(void)=0;
	virtual void _UNUSED_Key_LookupBinding(void)=0;
	virtual void _UNUSED_StartKeyTrapMode(void)=0;
	virtual void _UNUSED_CheckDoneKeyTrapping(void)=0;

	// Returns true if the player is fully connected and active in game (i.e, not still loading)
	virtual bool				IsInGame( void ) = 0;
	// Returns true if the player is connected, but not necessarily active in game (could still be loading)
	virtual bool				IsConnected( void ) = 0;
	// Returns true if the loading plaque should be drawn
	virtual bool				IsDrawingLoadingImage( void ) = 0;

	// Prints the formatted string to the notification area of the screen ( down the right hand edge
	//  numbered lines starting at position 0
	virtual void				Con_NPrintf( int pos, const char *fmt, ... ) = 0;
	
	virtual void _UNUSED_Con_NXPrintf(void)=0;

	// During ConCommand processing functions, use this function to get the total # of tokens passed to the command parser
	virtual int					Cmd_Argc( void ) = 0;	
	// During ConCommand processing, this API is used to access each argument passed to the parser
	virtual const char			*Cmd_Argv( int arg ) = 0;

	virtual void _UNUSED_IsBoxVisible(void)=0;
	virtual void _UNUSED_IsBoxInViewCluster(void)=0;
	virtual void _UNUSED_CullBox(void)=0;
	virtual void _UNUSED_Sound_ExtraUpdate(void)=0;

	// Get the current game directory ( e.g., hl2, tf2, cstrike, hl1 )
	virtual const char			*GetGameDirectory( void ) = 0;

	// Get access to the world to screen transformation matrix
	virtual const VMatrix& 		WorldToScreenMatrix() = 0;
	
	// Get the matrix to move a point from world space into view space
	// (translate and rotate so the camera is at the origin looking down X).
	virtual const VMatrix& 		WorldToViewMatrix() = 0;

	virtual void _UNUSED_GameLumpVersion(void)=0;
	virtual void _UNUSED_GameLumpSize(void)=0;
	virtual void _UNUSED_LoadGameLump(void)=0;
	virtual void _UNUSED_LevelLeafCount(void)=0;
	virtual void _UNUSED_GetBSPTreeQuery(void)=0;
	virtual void _UNUSED_LinearToGamma(void)=0;
	virtual void _UNUSED_LightStyleValue(void)=0;
	virtual void _UNUSED_ComputeDynamicLighting(void)=0;
	virtual void _UNUSED_GetAmbientLightColor(void)=0;
	virtual void _UNUSED_GetDXSupportLevel(void)=0;
	virtual void _UNUSED_SupportsHDR(void)=0;
	virtual void _UNUSED_Mat_Stub(void)=0;

	// Get the name of the current map
	virtual char const	*GetLevelName( void ) = 0;
#ifndef _XBOX
	virtual void _UNUSED_GetVoiceTweakAPI(void)=0;
#endif
	// Tell engine stats gathering system that the rendering frame is beginning/ending
	virtual void		EngineStats_BeginFrame( void ) = 0;
	virtual void		EngineStats_EndFrame( void ) = 0;
	
	virtual void _UNUSED_FireEvents(void)=0;
	virtual void _UNUSED_GetLeavesArea(void)=0;
	virtual void _UNUSED_DoesBoxTouchAreaFrustum(void)=0;
	virtual void _UNUSED_SetHearingOrigin(void)=0;
	virtual void _UNUSED_SentenceGroupPick(void)=0;
	virtual void _UNUSED_SentenceGroupPickSequential(void)=0;
	virtual void _UNUSED_SentenceIndexFromName(void)=0;
	virtual void _UNUSED_SentenceNameFromIndex(void)=0;
	virtual void _UNUSED_SentenceGroupIndexFromName(void)=0;
	virtual void _UNUSED_SentenceGroupNameFromIndex(void)=0;
	virtual void _UNUSED_SentenceLength(void)=0;
	virtual void _UNUSED_ComputeLighting(void)=0;
	virtual void _UNUSED_ActivateOccluder(void)=0;
	virtual void _UNUSED_IsOccluded(void)=0;
	virtual void _UNUSED_SaveAllocMemory(void)=0;
	virtual void _UNUSED_SaveFreeMemory(void)=0;
	virtual void _UNUSED_GetNetChannelInfo(void)=0;
	virtual void _UNUSED_DebugDrawPhysCollide(void)=0;
	virtual void _UNUSED_CheckPoint(void)=0;
	virtual void _UNUSED_DrawPortals(void)=0;

	// Determine whether the client is playing back or recording a demo
	virtual bool		IsPlayingDemo( void ) = 0;
	virtual bool		IsRecordingDemo( void ) = 0;
	virtual bool		IsPlayingTimeDemo( void ) = 0;
	// Is the game paused?
	virtual bool		IsPaused( void ) = 0;
	// Is the game currently taking a screenshot?
	virtual bool		IsTakingScreenshot( void ) = 0;
	// Is this a HLTV broadcast ?
	virtual bool		IsHLTV( void ) = 0;
	// is this level loaded as just the background to the main menu? (active, but unplayable)
	virtual bool		IsLevelMainMenuBackground( void ) = 0;
	// returns the name of the background level
	virtual void		GetMainMenuBackgroundName( char *dest, int destlen ) = 0;

	virtual void _UNUSED_SetOcclusionParameters(void)=0;
	virtual void _UNUSED_GetUILanguage(void)=0;
	virtual void _UNUSED_IsSkyboxVisibleFromPoint(void)=0;
	virtual void _UNUSED_GetMapEntitiesString(void)=0;

	// Is the engine in map edit mode ?
	virtual bool		IsInEditMode( void ) = 0;

	// current screen aspect ratio (eg. 4.0f/3.0f, 16.0f/9.0f)
	virtual float		GetScreenAspectRatio() = 0;

	virtual void _UNUSED_SteamRefreshLogin(void)=0;
	virtual void _UNUSED_SteamProcessCall(void)=0;

	// allow other modules to know about engine versioning (one use is a proxy for network compatability)
	virtual unsigned int	GetEngineBuildNumber() = 0; // engines build
	virtual const char *	GetProductVersionString() = 0; // mods version number (steam.inf)

	virtual void _UNUSED_GetLastPressedEngineKey(void)=0;
	virtual void _UNUSED_GrabPreColorCorrectedFrame(void)=0;

	virtual bool			IsHammerRunning( ) const = 0;

	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	// And then executes the command string immediately (vs ClientCmd() which executes in the next frame)
	virtual void			ExecuteClientCmd( const char *szCmdString ) = 0;

	virtual void _UNUSED_MapHasHDRLighting(void)=0;

	virtual int	GetAppID() = 0;

	virtual void _UNUSED_GetLightForPointFast(void)=0;
};


// IVEngineClient_013 //////////////////////////////////////////////////////////

#define VENGINE_CLIENT_INTERFACE_VERSION_013 "VEngineClient013"
#define VENGINE_CLIENT_INTERFACE_VERSION_015 "VEngineClient015"

/// <comments>
///		Supported by: Portal First Slice
///		</comments>
class IVEngineClient_013 abstract
{
public:
	virtual void _UNUSED_GetIntersectingSurfaces(void)=0;
	virtual void _UNUSED_GetLightForPoint(void)=0;
	virtual void _UNUSED_TraceLineMaterialAndLighting(void)=0;
	virtual void _UNUSED_ParseFile(void)=0;
	virtual void _UNUSED_CopyFile(void)=0;

	// Gets the dimensions of the game window
	virtual void				GetScreenSize( int& width, int& height ) = 0;

	// Forwards szCmdString to the server, sent reliably if bReliable is set
	virtual void				ServerCmd( const char *szCmdString, bool bReliable = true ) = 0;
	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	// Note: Calls to this are checked against FCVAR_CLIENTCMD_CAN_EXECUTE (if that bit is not set, then this function can't change it).
	//       Call ClientCmd_Unrestricted to have access to FCVAR_CLIENTCMD_CAN_EXECUTE vars.
	virtual void				ClientCmd( const char *szCmdString ) = 0;

	virtual void _UNUSED_GetPlayerInfo(void)=0;
	virtual void _UNUSED_GetPlayerForUserID(void)=0;
	virtual void _UNUSED_TextMessageGet(void)=0;

	// Returns true if the console is visible
	virtual bool				Con_IsVisible( void ) = 0;

	virtual void _UNUSED_GetLocalPlayer(void)=0;
	virtual void _UNUSED_LoadModel(void)=0;

	// Get accurate, sub-frame clock ( profiling use )
	virtual float				Time( void ) = 0; 

	// Get the exact server timesstamp ( server time ) from the last message received from the server
	virtual float				GetLastTimeStamp( void ) = 0; 

	virtual void _UNUSED_GetSentence(void)=0;
	virtual void _UNUSED_GetSentenceLength(void)=0;
	virtual void _UNUSED_IsStreaming(void)=0;

	// Copy current view orientation into va
	virtual void				GetViewAngles( QAngle& va ) = 0;
	// Set current view orientation from va
	virtual void				SetViewAngles( QAngle& va ) = 0;
	
	// Retrieve the current game's maxclients setting
	virtual int					GetMaxClients( void ) = 0;

	virtual void _UNUSED_Key_LookupBinding(void)=0;
	virtual void _UNUSED_Key_BindingForKey(void)=0;
	virtual void _UNUSED_StartKeyTrapMode(void)=0;
	virtual void _UNUSED_CheckDoneKeyTrapping(void)=0;

	// Returns true if the player is fully connected and active in game (i.e, not still loading)
	virtual bool				IsInGame( void ) = 0;
	// Returns true if the player is connected, but not necessarily active in game (could still be loading)
	virtual bool				IsConnected( void ) = 0;
	// Returns true if the loading plaque should be drawn
	virtual bool				IsDrawingLoadingImage( void ) = 0;

	// Prints the formatted string to the notification area of the screen ( down the right hand edge
	//  numbered lines starting at position 0
	virtual void				Con_NPrintf( int pos, const char *fmt, ... ) = 0;
	
	virtual void _UNUSED_Con_NXPrintf(void)=0;
	virtual void _UNUSED_IsBoxVisible(void)=0;
	virtual void _UNUSED_IsBoxInViewCluster(void)=0;
	virtual void _UNUSED_CullBox(void)=0;
	virtual void _UNUSED_Sound_ExtraUpdate(void)=0;

	// Get the current game directory ( e.g., hl2, tf2, cstrike, hl1 )
	virtual const char			*GetGameDirectory( void ) = 0;

	// Get access to the world to screen transformation matrix
	virtual const VMatrix& 		WorldToScreenMatrix() = 0;
	
	// Get the matrix to move a point from world space into view space
	// (translate and rotate so the camera is at the origin looking down X).
	virtual const VMatrix& 		WorldToViewMatrix() = 0;

	virtual void _UNUSED_GameLumpVersion(void)=0;
	virtual void _UNUSED_GameLumpSize(void)=0;
	virtual void _UNUSED_LoadGameLump(void)=0;
	virtual void _UNUSED_LevelLeafCount(void)=0;
	virtual void _UNUSED_GetBSPTreeQuery(void)=0;
	virtual void _UNUSED_LinearToGamma(void)=0;
	virtual void _UNUSED_LightStyleValue(void)=0;
	virtual void _UNUSED_ComputeDynamicLighting(void)=0;
	virtual void _UNUSED_GetAmbientLightColor(void)=0;
	virtual void _UNUSED_GetDXSupportLevel(void)=0;
	virtual void _UNUSED_SupportsHDR(void)=0;
	virtual void _UNUSED_Mat_Stub(void)=0;
	virtual void _UNUSED_GetChapterName(void)=0;

	virtual char const	*GetLevelName( void ) = 0;
	virtual int	GetLevelVersion( void ) = 0;

#if !defined( NO_VOICE )
	virtual void _UNUSED_GetVoiceTweakAPI(void)=0;
#endif
	// Tell engine stats gathering system that the rendering frame is beginning/ending
	virtual void		EngineStats_BeginFrame( void ) = 0;
	virtual void		EngineStats_EndFrame( void ) = 0;
	
	virtual void _UNUSED_FireEvents(void)=0;
	virtual void _UNUSED_GetLeavesArea(void)=0;
	virtual void _UNUSED_DoesBoxTouchAreaFrustum(void)=0;
	
	virtual void _UNUSED_SetAudioState(void)=0;
	virtual void _UNUSED_SentenceGroupPick(void)=0;
	virtual void _UNUSED_SentenceGroupPickSequential(void)=0;
	virtual void _UNUSED_SentenceIndexFromName(void)=0;
	virtual void _UNUSED_SentenceNameFromIndex(void)=0;
	virtual void _UNUSED_SentenceGroupIndexFromName(void)=0;
	virtual void _UNUSED_SentenceGroupNameFromIndex(void)=0;
	virtual void _UNUSED_SentenceLength(void)=0;
	virtual void _UNUSED_ComputeLighting(void)=0;
	virtual void _UNUSED_ActivateOccluder(void)=0;
	virtual void _UNUSED_IsOccluded(void)=0;
	virtual void _UNUSED_SaveAllocMemory(void)=0;
	virtual void _UNUSED_SaveFreeMemory(void)=0;
	virtual void _UNUSED_GetNetChannelInfo(void)=0;
	virtual void _UNUSED_DebugDrawPhysCollide(void)=0;
	virtual void _UNUSED_CheckPoint(void)=0;
	virtual void _UNUSED_DrawPortals(void)=0;

	// Determine whether the client is playing back or recording a demo
	virtual bool		IsPlayingDemo( void ) = 0;
	virtual bool		IsRecordingDemo( void ) = 0;
	virtual bool		IsPlayingTimeDemo( void ) = 0;

	// new in Source SDK 2013:
	virtual int			GetDemoRecordingTick( void ) = 0; 

	// new in Source SDK 2013:
 	virtual int			GetDemoPlaybackTick( void ) = 0; 

	// new in Source SDK 2013:
 	virtual int			GetDemoPlaybackStartTick( void ) = 0; 

	// new in Source SDK 2013:
 	virtual float		GetDemoPlaybackTimeScale( void ) = 0; 

	// new in Source SDK 2013:
 	virtual int			GetDemoPlaybackTotalTicks( void ) = 0; 


	// Is the game paused?
	virtual bool		IsPaused( void ) = 0;
	
	// Is the game currently taking a screenshot?
	virtual bool		IsTakingScreenshot( void ) = 0;
	
	// Is this a HLTV broadcast ?
	virtual bool		IsHLTV( void ) = 0;
	
	// is this level loaded as just the background to the main menu? (active, but unplayable)
	virtual bool		IsLevelMainMenuBackground( void ) = 0;
	// returns the name of the background level
	virtual void		GetMainMenuBackgroundName( char *dest, int destlen ) = 0;

	// new in Source SDK 2013:
	// Get video modes 
	virtual void _UNUSED_GetVideoModes(void) = 0; 

	virtual void _UNUSED_SetOcclusionParameters(void)=0;
	virtual void _UNUSED_GetUILanguage(void)=0;
	virtual void _UNUSED_IsSkyboxVisibleFromPoint(void)=0;
	virtual void _UNUSED_GetMapEntitiesString(void)=0;

	// Is the engine in map edit mode ?
	virtual bool		IsInEditMode( void ) = 0;

	// current screen aspect ratio (eg. 4.0f/3.0f, 16.0f/9.0f)
	virtual float		GetScreenAspectRatio() = 0;

	virtual void _UNUSED_REMOVED_SteamRefreshLogin(void)=0;
	virtual void _UNUSED_REMOVED_SteamProcessCall(void)=0;

	// allow other modules to know about engine versioning (one use is a proxy for network compatability)
	virtual unsigned int	GetEngineBuildNumber() = 0; // engines build
	virtual const char *	GetProductVersionString() = 0; // mods version number (steam.inf)

	virtual void _UNUSED_GrabPreColorCorrectedFrame(void)=0;

	virtual bool			IsHammerRunning( ) const = 0;

	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	// And then executes the command string immediately (vs ClientCmd() which executes in the next frame)
	//
	// Note: this is NOT checked against the FCVAR_CLIENTCMD_CAN_EXECUTE vars.
	virtual void			ExecuteClientCmd( const char *szCmdString ) = 0;

	virtual void _UNUSED_MapHasHDRLighting(void)=0;

	virtual int	GetAppID() = 0;

	virtual void _UNUSED_GetLightForPointFast(void)=0;

	// This version does NOT check against FCVAR_CLIENTCMD_CAN_EXECUTE.
	virtual void ClientCmd_Unrestricted( const char *szCmdString ) = 0;

	virtual void _UNUSED_SetRestrictServerCommands(void)=0;
	virtual void _UNUSED_SetRestrictClientCommands(void)=0;
	virtual void _UNUSED_SetOverlayBindProxy(void)=0;
	virtual void _UNUSED_CopyFrameBufferToMaterial(void)=0;
	virtual void _UNUSED_ChangeTeam(void)=0;
	virtual void _UNUSED_ReadConfiguration(void)=0;
	virtual void _UNUSED_SetAchievementMgr(void)=0;
	virtual void _UNUSED_GetAchievementMgr(void)=0;
	virtual void _UNUSED_MapLoadFailed(void)=0;
	virtual void _UNUSED_SetMapLoadFailed(void)=0;
	virtual void _UNUSED_IsLowViolence(void)=0;
	virtual void _UNUSED_GetMostRecentSaveGame(void)=0;
	virtual void _UNUSED_SetMostRecentSaveGame(void)=0;
	virtual void _UNUSED_StartXboxExitingProcess(void)=0;
	virtual void _UNUSED_IsSaveInProgress(void)=0;
	virtual void _UNUSED_OnStorageDeviceAttached(void)=0;
	virtual void _UNUSED_OnStorageDeviceDetached(void)=0;
	virtual void _UNUSED_ResetDemoInterpolation(void)=0;
	virtual void _UNUSED_SetGamestatsData(void)=0;
	virtual void _UNUSED_GetGamestatsData(void)=0;

	// .... might be more in some games (i.e. source-sdk-2013)
};

// IVEngineClient_014_csgo /////////////////////////////////////////////////////

#define VENGINE_CLIENT_INTERFACE_VERSION_014_CSGO "VEngineClient014"

/// <comments>
///		Supported by: CS:GO
///		</comments>
class IVEngineClient_014_csgo abstract
{
public:
	virtual void _UNUSED_GetIntersectingSurfaces(void)=0; // :0
	virtual void _UNUSED_GetLightForPoint(void)=0; // :1
	virtual void _UNUSED_TraceLineMaterialAndLighting(void)=0; // :2
	virtual void _UNUSED_ParseFile(void)=0; // :3
	virtual void _UNUSED_CopyFile(void)=0; // :4

	// Gets the dimensions of the game window
	virtual void				GetScreenSize( int& width, int& height ) = 0; // :5

	// Forwards szCmdString to the server, sent reliably if bReliable is set
	virtual void				ServerCmd( const char *szCmdString, bool bReliable = true ) = 0; // :6
	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	// Note: Calls to this are checked against FCVAR_CLIENTCMD_CAN_EXECUTE (if that bit is not set, then this function can't change it).
	//       Call ClientCmd_Unrestricted to have access to FCVAR_CLIENTCMD_CAN_EXECUTE vars.
	virtual void				ClientCmd( const char *szCmdString ) = 0; // :7

	virtual void _UNUSED_GetPlayerInfo(void)=0; // :8
	virtual void _UNUSED_GetPlayerForUserID(void)=0; // :9
	virtual void _UNUSED_TextMessageGet(void)=0; // :10

	// Returns true if the console is visible
	virtual bool				Con_IsVisible( void ) = 0; // :11

	virtual void _UNUSED_GetLocalPlayer(void)=0; // :12
	virtual void _UNUSED_LoadModel(void)=0; // :13

	// Get the exact server timesstamp ( server time ) from the last message received from the server
	virtual float				GetLastTimeStamp( void ) = 0;  // :14

	virtual void _UNUSED_GetSentence(void)=0; // :15
	virtual void _UNUSED_GetSentenceLength(void)=0; // :16
	virtual void _UNUSED_IsStreaming(void)=0; // :17

	// Copy current view orientation into va
	virtual void				GetViewAngles( QAngle& va ) = 0; // :18
	// Set current view orientation from va
	virtual void				SetViewAngles( QAngle& va ) = 0; // :19
	
	// Retrieve the current game's maxclients setting
	virtual int					GetMaxClients( void ) = 0; // :20

	virtual void _UNUSED_Key_LookupBinding(void)=0; // :21
	virtual void _UNUSED_Key_BindingForKey(void)=0; // :22

	virtual void _UNKOWN_023(void) = 0; // :23

	virtual void _UNUSED_StartKeyTrapMode(void)=0; // :24
	virtual void _UNUSED_CheckDoneKeyTrapping(void)=0; // :25

	// Returns true if the player is fully connected and active in game (i.e, not still loading)
	virtual bool				IsInGame( void ) = 0; // :26
	// Returns true if the player is connected, but not necessarily active in game (could still be loading)
	virtual bool				IsConnected( void ) = 0; // :27
	// Returns true if the loading plaque should be drawn
	virtual bool				IsDrawingLoadingImage( void ) = 0; // :28

	// new in csgo:
	virtual void				HideLoadingPlaque( void ) = 0; // :29

	// Prints the formatted string to the notification area of the screen ( down the right hand edge
	//  numbered lines starting at position 0
	virtual void				Con_NPrintf( int pos, const char *fmt, ... ) = 0; // :30
	
	virtual void _UNUSED_Con_NXPrintf(void)=0; // :31
	virtual void _UNUSED_IsBoxVisible(void)=0; // :32
	virtual void _UNUSED_IsBoxInViewCluster(void)=0; // :33
	virtual void _UNUSED_CullBox(void)=0; // :34
	virtual void _UNUSED_Sound_ExtraUpdate(void)=0; // :35

	// Get the current game directory ( e.g., hl2, tf2, cstrike, hl1 )
	virtual const char			*GetGameDirectory( void ) = 0; // :36

	// Get access to the world to screen transformation matrix
	virtual const VMatrix& 		WorldToScreenMatrix() = 0; // :37
	
	// Get the matrix to move a point from world space into view space
	// (translate and rotate so the camera is at the origin looking down X).
	virtual const VMatrix& 		WorldToViewMatrix() = 0; // :38

	virtual void _UNUSED_GameLumpVersion(void)=0; // :39
	virtual void _UNUSED_GameLumpSize(void)=0; // :40
	virtual void _UNUSED_LoadGameLump(void)=0; // :41
	virtual void _UNUSED_LevelLeafCount(void)=0; // :42
	virtual void _UNUSED_GetBSPTreeQuery(void)=0; // :43
	virtual void _UNUSED_LinearToGamma(void)=0; // :44
	virtual void _UNUSED_LightStyleValue(void)=0; // :45
	virtual void _UNUSED_ComputeDynamicLighting(void)=0; // :46
	virtual void _UNUSED_GetAmbientLightColor(void)=0; // :47
	virtual void _UNUSED_GetDXSupportLevel(void)=0; // :48
	virtual void _UNUSED_SupportsHDR(void)=0; // :49
	virtual void _UNUSED_Mat_Stub(void)=0; // :50
	virtual void _UNUSED_GetChapterName(void)=0; // :51

	virtual char const	*GetLevelName( void ) = 0; // :52

	virtual char const	*GetLevelNameShort(void) = 0; // :53

	// New in source SDK 2013
	virtual int GetLevelVersion( void ) = 0; // :54

#if !defined( NO_VOICE )
	virtual void _UNUSED_GetVoiceTweakAPI(void)=0; // :55
#endif
	// Tell engine stats gathering system that the rendering frame is beginning/ending
	virtual void		EngineStats_BeginFrame( void ) = 0; // :56
	virtual void		EngineStats_EndFrame( void ) = 0; // :57

	virtual void _UNKOWN_058(void); // :58
	
	virtual void _UNUSED_FireEvents(void)=0; // :59
	virtual void _UNUSED_GetLeavesArea(void)=0; // :60
	virtual void _UNUSED_DoesBoxTouchAreaFrustum(void)=0; // :61
	
	virtual void _UNKOWN_062(void) = 0; // :62

	virtual void _UNKOWN_063(void) = 0; // :63
	virtual void _UNKOWN_064(void) = 0; // :64

	virtual void _UNUSED_SentenceGroupPick(void)=0; // :65
	virtual void _UNUSED_SentenceGroupPickSequential(void)=0; // :66
	virtual void _UNUSED_SentenceIndexFromName(void)=0; // :67
	virtual void _UNUSED_SentenceNameFromIndex(void)=0; // :68
	virtual void _UNUSED_SentenceGroupIndexFromName(void)=0; // :69
	virtual void _UNUSED_SentenceGroupNameFromIndex(void)=0; // :70
	virtual void _UNUSED_SentenceLength(void)=0; // :71
	virtual void _UNUSED_ComputeLighting(void)=0; // :72

	virtual void _UNKOWN_073(void); // :73
	virtual void _UNKOWN_074(void); // :74
	virtual void _UNKOWN_075(void); // :75

	virtual void _UNUSED_SaveAllocMemory(void)=0; // :76
	virtual void _UNUSED_SaveFreeMemory(void)=0; // :77
	virtual void _UNUSED_GetNetChannelInfo(void)=0; // :78
	virtual void _UNUSED_DebugDrawPhysCollide(void)=0; // :79
	virtual void _UNUSED_CheckPoint(void)=0; // :80
	virtual void _UNUSED_DrawPortals(void)=0; // :81

	// Determine whether the client is playing back or recording a demo
	virtual bool		IsPlayingDemo( void ) = 0; // :82
	virtual bool		IsRecordingDemo( void ) = 0; // :83
	virtual bool		IsPlayingTimeDemo( void ) = 0; // :84

	// new in csgo:
	virtual int			GetDemoRecordingTick( void ) = 0; // :85
	virtual int			GetDemoPlaybackTick( void ) = 0; // :86
	virtual int			GetDemoPlaybackStartTick( void ) = 0; // :87
	virtual float		GetDemoPlaybackTimeScale( void ) = 0; // :88
	virtual int			GetDemoPlaybackTotalTicks( void ) = 0; // :89

	// Is the game paused?
	virtual bool		IsPaused( void ) = 0; // :90

	// What is the game timescale multiplied with the host_timescale?
	virtual float GetTimescale( void ) const = 0; // :91

	// Is the game currently taking a screenshot?
	virtual bool		IsTakingScreenshot( void ) = 0; // :92
	
	// Is this a HLTV broadcast ?
	virtual bool		IsHLTV( void ) = 0; // :93
	
	// is this level loaded as just the background to the main menu? (active, but unplayable)
	virtual bool		IsLevelMainMenuBackground( void ) = 0; // :94
	
	// returns the name of the background level
	virtual void		GetMainMenuBackgroundName( char *dest, int destlen ) = 0; // :95

	virtual void _UNUSED_SetOcclusionParameters(void)=0; // :96

	virtual void _UNUSED_GetUILanguage(void)=0; // :97


	virtual void _UNUSED_IsSkyboxVisibleFromPoint(void)=0; // :98
	virtual void _UNUSED_GetMapEntitiesString(void)=0; // :99

	// Is the engine in map edit mode ?
	virtual bool		IsInEditMode( void ) = 0; // :100

	// current screen aspect ratio (eg. 4.0f/3.0f, 16.0f/9.0f)
	virtual float		GetScreenAspectRatio(int viewportWidth, int viewportHeight) = 0; // :101

	virtual void _UNUSED_REMOVED_SteamRefreshLogin(void)=0; // :102
	virtual void _UNUSED_REMOVED_SteamProcessCall(void)=0; // :103

	// allow other modules to know about engine versioning (one use is a proxy for network compatability)
	virtual unsigned int	GetEngineBuildNumber() = 0; // engines build // :104
	virtual const char *	GetProductVersionString() = 0; // mods version number (steam.inf) // :105

	virtual void _UNUSED_GrabPreColorCorrectedFrame(void)=0; // :106

	virtual bool			IsHammerRunning( ) const = 0; // :107

	// Inserts szCmdString into the command buffer as if it was typed by the client to his/her console.
	// And then executes the command string immediately (vs ClientCmd() which executes in the next frame)
	//
	// Note: this is NOT checked against the FCVAR_CLIENTCMD_CAN_EXECUTE vars.
	virtual void			ExecuteClientCmd( const char *szCmdString ) = 0; // :108


	virtual void _UNKOWN_109(void); // :109
	virtual void _UNKOWN_110(void); // :110

	virtual int	GetAppID() = 0; // :111

	virtual void _UNKOWN_112(void); // :112
	virtual void _UNKOWN_113(void); // :113

	// This version does NOT check against FCVAR_CLIENTCMD_CAN_EXECUTE.
	virtual void ClientCmd_Unrestricted( const char *szCmdString ) = 0; // :114

	//
	// might be in-accurate:

	virtual void _UNUSED_SetRestrictServerCommands(void)=0;
	virtual void _UNUSED_SetRestrictClientCommands(void)=0;
	virtual void _UNUSED_SetOverlayBindProxy(void)=0;
	virtual void _UNUSED_CopyFrameBufferToMaterial(void)=0;
	virtual void _UNUSED_ReadConfiguration(void)=0;
	virtual void _UNUSED_SetAchievementMgr(void)=0;
	virtual void _UNUSED_GetAchievementMgr(void)=0;
	virtual void _UNUSED_MapLoadFailed(void)=0;
	virtual void _UNUSED_SetMapLoadFailed(void)=0;
	virtual void _UNUSED_IsLowViolence(void)=0;

	//
	// more we don't care about ...

};

// IBaseClientDLL_011 //////////////////////////////////////////////////////////

#define CLIENT_DLL_INTERFACE_VERSION_011		"VClient011"

class IBaseClientDLL_011 abstract
{
public:
	// Called once when the client DLL is loaded
	virtual int				Init( CreateInterfaceFn appSystemFactory, 
									CreateInterfaceFn physicsFactory,
									CGlobalVarsBase *pGlobals ) = 0; // 0

	// Called once when the client DLL is being unloaded
	virtual void			Shutdown( void ) = 0; // 1
	
	virtual void _UNUSED_LevelInitPreEntity(void)=0; // 2
	virtual void _UNUSED_LevelInitPostEntity(void)=0; // 3
	virtual void _UNUSED_LevelShutdown(void)=0; // 4
	virtual void _UNUSED_GetAllClasses(void)=0; // 5
	virtual void _UNUSED_HudVidInit(void)=0; // 6
	virtual void _UNUSED_HudProcessInput(void)=0; // 7
	virtual void _UNUSED_HudUpdate(void)=0; // 8
	virtual void _UNUSED_HudReset(void)=0; // 9
	virtual void _UNUSED_HudText(void)=0; // 10
	virtual void _UNUSED_IN_ActivateMouse(void)=0; // 11
	virtual void _UNUSED_IN_DeactivateMouse(void)=0; // 12
	virtual void _UNUSED_IN_MouseEvent(void)=0; // 13
	virtual void _UNUSED_IN_Accumulate(void)=0; // 14
	virtual void _UNUSED_IN_ClearStates(void)=0; // 15
	virtual void _UNUSED_IN_IsKeyDown(void)=0; // 16
	virtual void _UNUSED_IN_KeyEvent(void)=0; // 17
	virtual void _UNUSED_CreateMove(void)=0; // 18
	virtual void _UNUSED_ExtraMouseSample(void)=0; // 19
	virtual void _UNUSED_WriteUsercmdDeltaToBuffer(void)=0; // 20
	virtual void _UNUSED_EncodeUserCmdToBuffer(void)=0; // 21
	virtual void _UNUSED_DecodeUserCmdFromBuffer(void)=0; // 22
	virtual void _UNUSED_View_Render(void)=0; // 23
	virtual void _UNUSED_RenderView(void)=0; // 24
	virtual void _UNUSED_View_Fade(void)=0;
	virtual void _UNUSED_SetCrosshairAngle(void)=0;
	virtual void _UNUSED_InitSprite(void)=0;
	virtual void _UNUSED_ShutdownSprite(void)=0;
	virtual void _UNUSED_GetSpriteSize(void)=0;
	virtual void _UNUSED_VoiceStatus(void)=0;
	virtual void _UNUSED_InstallStringTableCallback(void)=0;
	virtual void _UNUSED_FrameStageNotify(void)=0;
	virtual void _UNUSED_DispatchUserMessage(void)=0;
	virtual void _UNUSED_SaveInit(void)=0;
	virtual void _UNUSED_SaveWriteFields(void)=0;
	virtual void _UNUSED_SaveReadFields(void)=0;
	virtual void _UNUSED_PreSave(void)=0;
	virtual void _UNUSED_Save(void)=0;
	virtual void _UNUSED_WriteSaveHeaders(void)=0;
	virtual void _UNUSED_ReadRestoreHeaders(void)=0;
	virtual void _UNUSED_Restore(void)=0;
	virtual void _UNUSED_DispatchOnRestore(void)=0;
	virtual void _UNUSED_GetStandardRecvProxies(void)=0;
	virtual void _UNUSED_WriteSaveGameScreenshot(void)=0;
	virtual void _UNUSED_EmitSentenceCloseCaption(void)=0;
	virtual void _UNUSED_EmitCloseCaption(void)=0;
};


// IBaseClientDLL_012 //////////////////////////////////////////////////////////

#define CLIENT_DLL_INTERFACE_VERSION_012		"VClient012"

/// <comments>
///		Supported by:
///		</comments>
class IBaseClientDLL_012 abstract
{
public:
	// Called once when the client DLL is loaded
	virtual int				Init( CreateInterfaceFn appSystemFactory, 
									CreateInterfaceFn physicsFactory,
									CGlobalVarsBase *pGlobals ) = 0;

	// Called once when the client DLL is being unloaded
	virtual void			Shutdown( void ) = 0;
	
	virtual void _UNUSED_LevelInitPreEntity(void)=0;
	virtual void _UNUSED_LevelInitPostEntity(void)=0;
	virtual void _UNUSED_LevelShutdown(void)=0;
	virtual void _UNUSED_GetAllClasses(void)=0;
	virtual void _UNUSED_HudVidInit(void)=0;
	virtual void _UNUSED_HudProcessInput(void)=0;
	virtual void _UNUSED_HudUpdate(void)=0;
	virtual void _UNUSED_HudReset(void)=0;
	virtual void _UNUSED_HudText(void)=0;
	virtual void _UNUSED_IN_ActivateMouse(void)=0;
	virtual void _UNUSED_IN_DeactivateMouse(void)=0;
	virtual void _UNUSED_IN_MouseEvent(void)=0;
	virtual void _UNUSED_IN_Accumulate(void)=0;
	virtual void _UNUSED_IN_ClearStates(void)=0;
	virtual void _UNUSED_IN_IsKeyDown(void)=0;
	virtual void _UNUSED_IN_KeyEvent(void)=0;
	virtual void _UNUSED_CreateMove (void)=0;
	virtual void _UNUSED_ExtraMouseSample(void)=0;
	virtual void _UNUSED_WriteUsercmdDeltaToBuffer(void)=0;
	virtual void _UNUSED_EncodeUserCmdToBuffer(void)=0;
	virtual void _UNUSED_DecodeUserCmdFromBuffer(void)=0;
	virtual void _UNUSED_View_Render(void)=0;
	virtual void _UNUSED_RenderView(void)=0;
	virtual void _UNUSED_View_Fade(void)=0;
	virtual void _UNUSED_InitSprite(void)=0;
	virtual void _UNUSED_ShutdownSprite(void)=0;
	virtual void _UNUSED_GetSpriteSize(void)=0;
	virtual void _UNUSED_VoiceStatus(void)=0;
	virtual void _UNUSED_InstallStringTableCallback(void)=0;
	virtual void _UNUSED_FrameStageNotify(void)=0;
	virtual void _UNUSED_DispatchUserMessage(void)=0;
	virtual void _UNUSED_SaveInit(void)=0;
	virtual void _UNUSED_SaveWriteFields(void)=0;
	virtual void _UNUSED_SaveReadFields(void)=0;
	virtual void _UNUSED_PreSave(void)=0;
	virtual void _UNUSED_Save(void)=0;
	virtual void _UNUSED_WriteSaveHeaders(void)=0;
	virtual void _UNUSED_ReadRestoreHeaders(void)=0;
	virtual void _UNUSED_Restore(void)=0;
	virtual void _UNUSED_DispatchOnRestore(void)=0;
	virtual void _UNUSED_GetStandardRecvProxies(void)=0;
	virtual void _UNUSED_WriteSaveGameScreenshot(void)=0;
	virtual void _UNUSED_EmitSentenceCloseCaption(void)=0;
	virtual void _UNUSED_EmitCloseCaption(void)=0;
	virtual void _UNUSED_CanRecordDemo(void)=0;
};


// IBaseClientDLL_013 //////////////////////////////////////////////////////////

#define CLIENT_DLL_INTERFACE_VERSION_013 "VClient013"

/// <comments>
///		Supported by: Counter-Strike Source
///		</comments>
class IBaseClientDLL_013 abstract
{
public:
	// Called once when the client DLL is loaded
	virtual int				Init( CreateInterfaceFn appSystemFactory, 
									CreateInterfaceFn physicsFactory,
									CGlobalVarsBase *pGlobals ) = 0; // 0

	// Called once when the client DLL is being unloaded
	virtual void			Shutdown( void ) = 0; // 1
	
	virtual void _UNUSED_LevelInitPreEntity(void)=0; // 2
	virtual void _UNUSED_LevelInitPostEntity(void)=0; // 3
	virtual void _UNUSED_LevelShutdown(void)=0; // 4
	virtual void _UNUSED_GetAllClasses(void)=0; // 5
	virtual void _UNUSED_HudVidInit(void)=0; // 6
	virtual void _UNUSED_HudProcessInput(void)=0; // 7
	virtual void _UNUSED_HudUpdate(void)=0; // 8
	virtual void _UNUSED_HudReset(void)=0; // 9
	virtual void _UNUSED_HudText(void)=0; // 10
	virtual void _UNUSED_IN_ActivateMouse(void)=0; // 11
	virtual void _UNUSED_IN_DeactivateMouse(void)=0; // 12
	virtual void _UNUSED_IN_MouseEvent (void)=0; // 13
	virtual void _UNUSED_IN_Accumulate (void)=0; // 14
	virtual void _UNUSED_IN_ClearStates (void)=0; // 15
	virtual void _UNUSED_IN_IsKeyDown(void)=0; // 16
	virtual void _UNUSED_IN_KeyEvent(void)=0; // 17
	virtual void _UNUSED_CreateMove(void)=0; // 18
	virtual void _UNUSED_ExtraMouseSample(void)=0; // 19
	virtual void _UNUSED_WriteUsercmdDeltaToBuffer(void)=0; // 20
	virtual void _UNUSED_EncodeUserCmdToBuffer(void)=0; // 21
	virtual void _UNUSED_DecodeUserCmdFromBuffer(void)=0; // 22
	virtual void _UNUSED_View_Render(void)=0; // 23
	virtual void _UNUSED_RenderView(void)=0; // 24
	virtual void _UNUSED_View_Fade(void)=0; // 25
	virtual void _UNUSED_SetCrosshairAngle(void)=0; // 26
	virtual void _UNUSED_InitSprite(void)=0; // 27
	virtual void _UNUSED_ShutdownSprite(void)=0; // 28
	virtual void _UNUSED_GetSpriteSize(void)=0; // 29
	virtual void _UNUSED_VoiceStatus(void)=0; // 30
	virtual void _UNUSED_InstallStringTableCallback(void)=0; // 31
	virtual void _UNUSED_FrameStageNotify(void)=0; // 32
	virtual void _UNUSED_DispatchUserMessage(void)=0; // 33
	virtual void _UNUSED_SaveInit(void)=0; // 34
	virtual void _UNUSED_SaveWriteFields(void)=0; // 35
	virtual void _UNUSED_SaveReadFields(void)=0; // 36
	virtual void _UNUSED_PreSave(void)=0; // 37
	virtual void _UNUSED_Save(void)=0; // 38
	virtual void _UNUSED_WriteSaveHeaders(void)=0; // 39
	virtual void _UNUSED_ReadRestoreHeaders(void)=0; // 40
	virtual void _UNUSED_Restore(void)=0; // 41
	virtual void _UNUSED_DispatchOnRestore(void)=0; // 42
	virtual void _UNUSED_GetStandardRecvProxies(void)=0; // 43
	virtual void _UNUSED_WriteSaveGameScreenshot(void)=0; // 44
	virtual void _UNUSED_EmitSentenceCloseCaption(void)=0; // 45
	virtual void _UNUSED_EmitCloseCaption(void)=0; // 46
	virtual void _UNUSED_CanRecordDemo(void)=0; // 47
	virtual void _UNUSED_WriteSaveGameScreenshotOfSize(void)=0; // 48
	virtual void _UNUSED_RenderViewEx(void)=0; // 49
	virtual void _UNUSED_GetPlayerView(void)=0; // 50
};


// IBaseClientDll_015 //////////////////////////////////////////////////////////

#define CLIENT_DLL_INTERFACE_VERSION_015		"VClient015"

class CGlobalVarsBase;

/// <comments>
///		Supported by: Portal First Slice
///		</comments>
class IBaseClientDLL_015 abstract
{
public:
	// Called once when the client DLL is loaded
	virtual int				Init( CreateInterfaceFn appSystemFactory, 
									CreateInterfaceFn physicsFactory,
									CGlobalVarsBase *pGlobals ) = 0;

	virtual void			PostInit() = 0;

	// Called once when the client DLL is being unloaded
	virtual void			Shutdown( void ) = 0;
	
	virtual void _UNUSED_LevelInitPreEntity(void)=0;
	virtual void _UNUSED_LevelInitPostEntity(void)=0;
	virtual void _UNUSED_LevelShutdown(void)=0;
	virtual void _UNUSED_GetAllClasses(void)=0;
	virtual void _UNUSED_HudVidInit(void)=0;
	virtual void _UNUSED_HudProcessInput(void)=0;
	virtual void _UNUSED_HudUpdate(void)=0;
	virtual void _UNUSED_HudReset(void)=0;
	virtual void _UNUSED_HudText(void)=0;
	virtual void _UNUSED_IN_ActivateMouse(void)=0;
	virtual void _UNUSED_IN_DeactivateMouse(void)=0;
	virtual void _UNUSED_IN_Accumulate(void)=0;
	virtual void _UNUSED_IN_ClearStates(void)=0;
	virtual void _UNUSED_IN_IsKeyDown(void)=0;
	virtual void _UNUSED_IN_KeyEvent(void)=0;
	virtual void _UNUSED_CreateMove(void)=0;
	virtual void _UNUSED_ExtraMouseSample(void)=0;
	virtual void _UNUSED_WriteUsercmdDeltaToBuffer(void)=0;
	virtual void _UNUSED_EncodeUserCmdToBuffer(void)=0;
	virtual void _UNUSED_DecodeUserCmdFromBuffer(void)=0;
	virtual void _UNUSED_View_Render(void)=0;
	virtual void _UNUSED_RenderView(void)=0;
	virtual void _UNUSED_View_Fade(void)=0;
	virtual void _UNUSED_SetCrosshairAngle(void)=0;
	virtual void _UNUSED_InitSprite(void)=0;
	virtual void _UNUSED_ShutdownSprite(void)=0;
	virtual void _UNUSED_GetSpriteSize(void)=0;
	virtual void _UNUSED_VoiceStatus(void)=0;
	virtual void _UNUSED_InstallStringTableCallback(void)=0;
	virtual void _UNUSED_FrameStageNotify(void)=0;
	virtual void _UNUSED_DispatchUserMessage(void)=0;
	virtual void _UNUSED_SaveInit(void)=0;
	virtual void _UNUSED_SaveWriteFields(void)=0;
	virtual void _UNUSED_SaveReadFields(void)=0;
	virtual void _UNUSED_PreSave(void)=0;
	virtual void _UNUSED_Save(void)=0;
	virtual void _UNUSED_WriteSaveHeaders(void)=0;
	virtual void _UNUSED_ReadRestoreHeaders(void)=0;
	virtual void _UNUSED_Restore(void)=0;
	virtual void _UNUSED_DispatchOnRestore(void)=0;
	virtual void _UNUSED_GetStandardRecvProxies(void)=0;
	virtual void _UNUSED_WriteSaveGameScreenshot(void)=0;
	virtual void _UNUSED_EmitSentenceCloseCaption(void)=0;
	virtual void _UNUSED_EmitCloseCaption(void)=0;
	virtual void _UNUSED_CanRecordDemo(void)=0;
	virtual void _UNUSED_WriteSaveGameScreenshotOfSize(void)=0;
	virtual void _UNUSED_GetPlayerView(void)=0;
	virtual void _UNUSED_SetupGameProperties(void)=0;
	virtual void _UNUSED_GetPresenceID(void)=0;
	virtual void _UNUSED_GetPropertyIdString(void)=0;
	virtual void _UNUSED_GetPropertyDisplayString(void)=0;
#ifdef _WIN32
	virtual void _UNUSED_StartStatsReporting(void)=0;
#endif
	virtual void _UNUSED_InvalidateMdlCache(void)=0;
	virtual void _UNUSED_IN_SetSampleTime(void)=0;
};

// IBaseClientDll_016 //////////////////////////////////////////////////////////

#define CLIENT_DLL_INTERFACE_VERSION_016		"VClient016" 

// IBaseClientDll_017 //////////////////////////////////////////////////////////

#define CLIENT_DLL_INTERFACE_VERSION_017		"VClient017"

// IBaseClientDll_018 //////////////////////////////////////////////////////////

#define CLIENT_DLL_INTERFACE_VERSION_018		"VClient018"

// IBaseInterface //////////////////////////////////////////////////////////////

class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};

// IClientEngineTools //////////////////////////////////////////////////////////

typedef void * HTOOLHANDLE;
typedef void KeyValues_something;
typedef struct {} AudioState_t;

class IClientEngineTools_001 : public IBaseInterface
{
public:
	// Level init, shutdown
	virtual void LevelInitPreEntityAllTools() = 0;
	// entities are created / spawned / precached here
	virtual void LevelInitPostEntityAllTools() = 0;

	virtual void LevelShutdownPreEntityAllTools() = 0;
	// Entities are deleted / released here...
	virtual void LevelShutdownPostEntityAllTools() = 0;

	virtual void PreRenderAllTools() = 0;
	virtual void PostRenderAllTools() = 0;

	virtual void PostToolMessage( HTOOLHANDLE hEntity, KeyValues_something *msg ) = 0;

	virtual void AdjustEngineViewport( int& x, int& y, int& width, int& height ) = 0;
	virtual bool SetupEngineView( Vector &origin, QAngle &angles, float &fov ) = 0;
	virtual bool SetupAudioState( AudioState_t &audioState ) = 0;

	// Paintmode is an enum declared in ienginevgui.h
	virtual void VGui_PreRenderAllTools( int paintMode ) = 0;
	virtual void VGui_PostRenderAllTools( int paintMode ) = 0;

	virtual bool IsThirdPersonCamera( ) = 0;

	virtual bool InToolMode() = 0;
};

#define VCLIENTENGINETOOLS_INTERFACE_VERSION_001 "VCLIENTENGINETOOLS001"

// IMaterial_csgo //////////////////////////////////////////////////////////////

class IMaterialVar_csgo;

class IMaterial_csgo abstract
{
public:
	// 000:
	virtual const char *	GetName() const = 0;

	// 001:
	virtual const char *	GetTextureGroupName() const = 0;

	virtual void _UNKNOWN_002(void) = 0;
	virtual void _UNKNOWN_003(void) = 0;
	virtual void _UNKNOWN_004(void) = 0;
	virtual void _UNKNOWN_005(void) = 0;
	virtual void _UNKNOWN_006(void) = 0;
	virtual void _UNKNOWN_007(void) = 0;
	virtual void _UNKNOWN_008(void) = 0;
	virtual void _UNKNOWN_009(void) = 0;
	virtual void _UNKNOWN_010(void) = 0;
	virtual void _UNKNOWN_011(void) = 0;

	// 012:
	virtual void IncrementReferenceCount( void ) = 0;

	// 013:
	virtual void DecrementReferenceCount( void ) = 0;

	virtual void _UNKNOWN_014(void) = 0;
	virtual void _UNKNOWN_015(void) = 0;
	virtual void _UNKNOWN_016(void) = 0;
	virtual void _UNKNOWN_017(void) = 0;
	virtual void _UNKNOWN_018(void) = 0;
	virtual void _UNKNOWN_019(void) = 0;

	virtual void _UNKNOWN_020(void) = 0;
	virtual void _UNKNOWN_021(void) = 0;
	virtual void _UNKNOWN_022(void) = 0;
	virtual void _UNKNOWN_023(void) = 0;
	virtual void _UNKNOWN_024(void) = 0;
	virtual void _UNKNOWN_025(void) = 0;
	virtual void _UNKNOWN_026(void) = 0;
	virtual void _UNKNOWN_027(void) = 0;
	virtual void _UNKNOWN_028(void) = 0;
	virtual void _UNKNOWN_029(void) = 0;

	virtual void _UNKNOWN_030(void) = 0;
	virtual void _UNKNOWN_031(void) = 0;
	virtual void _UNKNOWN_032(void) = 0;
	virtual void _UNKNOWN_033(void) = 0;
	virtual void _UNKNOWN_034(void) = 0;
	virtual void _UNKNOWN_035(void) = 0;
	virtual void _UNKNOWN_036(void) = 0;
	virtual void _UNKNOWN_037(void) = 0;
	virtual void _UNKNOWN_038(void) = 0;
	virtual void _UNKNOWN_039(void) = 0;

	// 040:
	virtual int ShaderParamCount() const = 0;

	// 041:
	virtual IMaterialVar_csgo **GetShaderParams( void ) = 0;

	
	// 042:
	// Returns true if this is the error material you get back from IMaterialSystem::FindMaterial if
	// the material can't be found.
	virtual bool IsErrorMaterial() const = 0;

	virtual void _UNKNOWN_043(void) = 0;
	virtual void _UNKNOWN_044(void) = 0;
	virtual void _UNKNOWN_045(void) = 0;
	virtual void _UNKNOWN_046(void) = 0;
	virtual void _UNKNOWN_047(void) = 0;
	virtual void _UNKNOWN_048(void) = 0;

	// 049:
	virtual const char * GetShaderName() const = 0;

	// ...
	// there are more here that we don't care about.
};

// IRefCounted_csgo ////////////////////////////////////////////////////////////

class IRefCounted_csgo abstract
{
public:
	virtual int AddRef() = 0;
	virtual int Release() = 0;
};

// IMesh_csgo //////////////////////////////////////////////////////////////////

typedef unsigned __int8 uint8;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

typedef uint64 VertexFormat_t_csgo;

typedef void MeshBoneRemap_t_csgo;
typedef void matrix3x4_t_csgo;
typedef void MaterialLightingState_t_csgo;

class ITexture_csgo
{
public:
	virtual void _UNKNOWN_000(void) = 0;
	virtual void _UNKNOWN_001(void) = 0;
	virtual void _UNKNOWN_002(void) = 0;
	virtual void _UNKNOWN_003(void) = 0;
	virtual void _UNKNOWN_004(void) = 0;
	virtual void _UNKNOWN_005(void) = 0;
	virtual void _UNKNOWN_006(void) = 0;
	virtual void _UNKNOWN_007(void) = 0;
	virtual void _UNKNOWN_008(void) = 0;
	virtual void _UNKNOWN_009(void) = 0;

	// Methods associated with reference count
	virtual void IncrementReferenceCount( void ) = 0;
	virtual void DecrementReferenceCount( void ) = 0;

	// ...
	// more we don't care about;
};

enum MaterialPrimitiveType_t_csgo 
{ 
	MATERIAL_POINTS			= 0x0,
	MATERIAL_LINES,
	MATERIAL_TRIANGLES,
	MATERIAL_TRIANGLE_STRIP,
	MATERIAL_LINE_STRIP,
	MATERIAL_LINE_LOOP,	// a single line loop
	MATERIAL_POLYGON,	// this is a *single* polygon
	MATERIAL_QUADS,
	MATERIAL_SUBD_QUADS_EXTRA, // Extraordinary sub-d quads
	MATERIAL_SUBD_QUADS_REG,   // Regular sub-d quads
	MATERIAL_INSTANCED_QUADS, // (X360) like MATERIAL_QUADS, but uses vertex instancing

	// This is used for static meshes that contain multiple types of
	// primitive types.	When calling draw, you'll need to specify
	// a primitive type.
	MATERIAL_HETEROGENOUS
};


typedef void ShaderStencilState_t_csgo;
typedef int MaterialIndexFormat_t_csgo;


class IMesh_csgo;

// For now, vertex compression is simply "on or off" (for the sake of simplicity
// and MeshBuilder perf.), but later we may support multiple flavours.
enum VertexCompressionType_t_csgo
{
	// This indicates an uninitialized VertexCompressionType_t value
	VERTEX_COMPRESSION_INVALID = 0xFFFFFFFF,

	// 'VERTEX_COMPRESSION_NONE' means that no elements of a vertex are compressed
	VERTEX_COMPRESSION_NONE = 0,

	// Currently (more stuff may be added as needed), 'VERTEX_COMPRESSION_ON' means:
	//  - if a vertex contains VERTEX_ELEMENT_NORMAL, this is compressed
	//    (see CVertexBuilder::CompressedNormal3f)
	//  - if a vertex contains VERTEX_ELEMENT_USERDATA4 (and a normal - together defining a tangent
	//    frame, with the binormal reconstructed in the vertex shader), this is compressed
	//    (see CVertexBuilder::CompressedUserData)
	//  - if a vertex contains VERTEX_ELEMENT_BONEWEIGHTSx, this is compressed
	//    (see CVertexBuilder::CompressedBoneWeight3fv)
	VERTEX_COMPRESSION_ON = 1
};

enum
{
	VERTEX_MAX_TEXTURE_COORDINATES_csgo = 8,
	BONE_MATRIX_INDEX_INVALID_csgo = 255
};

struct VertexDesc_t_csgo
{
	// These can be set to zero if there are pointers to dummy buffers, when the
	// actual buffer format doesn't contain the data but it needs to be safe to
	// use all the CMeshBuilder functions.
	int	m_VertexSize_Position;
	int m_VertexSize_BoneWeight;
	int m_VertexSize_BoneMatrixIndex;
	int	m_VertexSize_Normal;
	int	m_VertexSize_Color;
	int	m_VertexSize_Specular;
	int m_VertexSize_TexCoord[VERTEX_MAX_TEXTURE_COORDINATES_csgo];
	int m_VertexSize_TangentS;
	int m_VertexSize_TangentT;
	int m_VertexSize_Wrinkle;

	int m_VertexSize_UserData;

	int m_ActualVertexSize;	// Size of the vertices.. Some of the m_VertexSize_ elements above
							// are set to this value and some are set to zero depending on which
							// fields exist in a buffer's vertex format.

	// The type of compression applied to this vertex data
	VertexCompressionType_t_csgo m_CompressionType;

	// Number of bone weights per vertex...
	int m_NumBoneWeights;

	// Pointers to our current vertex data
	float			*m_pPosition;

	float			*m_pBoneWeight;

#ifndef NEW_SKINNING_csgo
	unsigned char	*m_pBoneMatrixIndex;
#else
	float			*m_pBoneMatrixIndex;
#endif

	float			*m_pNormal;

	unsigned char	*m_pColor;
	unsigned char	*m_pSpecular;
	float			*m_pTexCoord[VERTEX_MAX_TEXTURE_COORDINATES_csgo];

	// Tangent space *associated with one particular set of texcoords*
	float			*m_pTangentS;
	float			*m_pTangentT;

	float			*m_pWrinkle;

	// user data
	float			*m_pUserData;

	// The first vertex index (used for buffered vertex buffers, or cards that don't support stream offset)
	int	m_nFirstVertex;

	// The offset in bytes of the memory we're writing into 
	// from the start of the D3D buffer (will be 0 for static meshes)
	unsigned int	m_nOffset;

#ifdef DEBUG_WRITE_COMBINE_csgo
	int m_nLastWrittenField;
	unsigned char* m_pLastWrittenAddress;
#endif
};

struct IndexDesc_t_csgo
{
	// Pointers to the index data
	unsigned short	*m_pIndices;

	// The offset in bytes of the memory we're writing into 
	// from the start of the D3D buffer (will be 0 for static meshes)
	unsigned int	m_nOffset;

	// The first index (used for buffered index buffers, or cards that don't support stream offset)
	unsigned int	m_nFirstIndex;

	// 1 if the device is active, 0 if the device isn't active.
	// Faster than doing if checks for null m_pIndices if someone is
	// trying to write the m_pIndices while the device is inactive.
	unsigned int	m_nIndexSize;
};

struct MeshDesc_t_csgo : public VertexDesc_t_csgo, public IndexDesc_t_csgo
{
};

struct MeshBuffersAllocationSettings_t_csgo
{
	uint32 m_uiIbUsageFlags;
};

class Vector4D_csgo					
{
public:
	// Members
	vec_t x, y, z, w;
};

class IVertexBuffer_csgo abstract
{
public:
	// NOTE: The following two methods are only valid for static vertex buffers
	// Returns the number of vertices and the format of the vertex buffer
	virtual int VertexCount() const = 0; // :000
	virtual VertexFormat_t_csgo GetVertexFormat() const = 0; // :001

	// Is this vertex buffer dynamic?
	virtual bool IsDynamic() const = 0; // :002

	// NOTE: For dynamic vertex buffers only!
	// Casts the memory of the dynamic vertex buffer to the appropriate type
	virtual void BeginCastBuffer( VertexFormat_t_csgo format ) = 0; // :003
	virtual void EndCastBuffer() = 0; // :004

	// Returns the number of vertices that can still be written into the buffer
	virtual int GetRoomRemaining() const = 0; // :005

	virtual bool Lock( int nVertexCount, bool bAppend, VertexDesc_t_csgo &desc ) = 0; // :006*
	virtual void Unlock( int nVertexCount, VertexDesc_t_csgo &desc ) = 0; // :007*

	// Spews the mesh data
	virtual void Spew( int nVertexCount, const VertexDesc_t_csgo &desc ) = 0;  // :008

	// Call this in debug mode to make sure our data is good.
	virtual void ValidateData( int nVertexCount, const VertexDesc_t_csgo & desc ) = 0; // :009

	virtual void _Unknown_10_IVertexBuffer_csgo(void) = 0; // :010
};

class IIndexBuffer_csgo abstract
{
public:
	// NOTE: The following two methods are only valid for static index buffers
	// Returns the number of indices and the format of the index buffer
	virtual int IndexCount() const = 0;
	virtual MaterialIndexFormat_t_csgo IndexFormat() const = 0;

	// Is this index buffer dynamic?
	virtual bool IsDynamic() const = 0;

	// NOTE: For dynamic index buffers only!
	// Casts the memory of the dynamic index buffer to the appropriate type
	virtual void BeginCastBuffer( MaterialIndexFormat_t_csgo format ) = 0;
	virtual void EndCastBuffer() = 0;

	// Returns the number of indices that can still be written into the buffer
	virtual int GetRoomRemaining() const = 0;

	// Locks, unlocks the index buffer
	virtual bool Lock( int nMaxIndexCount, bool bAppend, IndexDesc_t_csgo &desc ) = 0;
	virtual void Unlock( int nWrittenIndexCount, IndexDesc_t_csgo &desc ) = 0;

	// FIXME: Remove this!! Here only for backward compat on IMesh
	// Locks, unlocks the index buffer for modify
	virtual void ModifyBegin( bool bReadOnly, int nFirstIndex, int nIndexCount, IndexDesc_t_csgo& desc ) = 0;
	virtual void ModifyEnd( IndexDesc_t_csgo& desc ) = 0;

	// Spews the mesh data
	virtual void Spew( int nIndexCount, const IndexDesc_t_csgo &desc ) = 0;

	// Ensures the data in the index buffer is valid
	virtual void ValidateData( int nIndexCount, const IndexDesc_t_csgo &desc ) = 0;

	// For backward compat to IMesh
	virtual IMesh_csgo* GetMesh() = 0;

	//
	// Those are probably not required anymore, but better safe than sorry:

	virtual void _Unknown_13_IIndexBuffer_csgo(void * arg0) = 0;

	virtual bool _Unknown_14_IIndexBuffer_csgo(void) = 0;
};

class CPrimList_csgo;
class CMeshBuilder_csgo;
class ICachedPerFrameMeshData_csgo;

class IMesh_csgo abstract : public IVertexBuffer_csgo, public IIndexBuffer_csgo
{
public:
	// -----------------------------------

	// Draws the mesh
	virtual void Draw( int firstIndex = -1, int numIndices = 0 ) = 0; // :012

	virtual void SetColorMesh( IMesh_csgo *pColorMesh, int nVertexOffset ) = 0; // :013

	// Draw a list of (lists of) primitives. Batching your lists together that use
	// the same lightmap, material, vertex and index buffers with multipass shaders
	// can drastically reduce state-switching overhead.
	// NOTE: this only works with STATIC meshes.
	virtual void Draw( CPrimList_csgo *pLists, int nLists ) = 0; // :011

	// Copy verts and/or indices to a mesh builder. This only works for temp meshes!
	virtual void CopyToMeshBuilder( 
		int iStartVert,		// Which vertices to copy.
		int nVerts, 
		int iStartIndex,	// Which indices to copy.
		int nIndices, 
		int indexOffset,	// This is added to each index.
		CMeshBuilder_csgo &builder ) = 0; // :014

	// Spews the mesh data
	virtual void Spew( int numVerts, int numIndices, const MeshDesc_t_csgo &desc ) = 0; // :015

	// Call this in debug mode to make sure our data is good.
	virtual void ValidateData( int numVerts, int numIndices, const MeshDesc_t_csgo &desc ) = 0; // :016

	// New version
	// Locks/unlocks the mesh, providing space for numVerts and numIndices.
	// numIndices of -1 means don't lock the index buffer...
	virtual void LockMesh( int numVerts, int numIndices, MeshDesc_t_csgo &desc, MeshBuffersAllocationSettings_t_csgo *pSettings ) = 0; // :017
	virtual void ModifyBegin( int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t_csgo& desc ) = 0; // :018
	virtual void ModifyEnd( MeshDesc_t_csgo& desc ) = 0; // :019
	virtual void UnlockMesh( int numVerts, int numIndices, MeshDesc_t_csgo &desc ) = 0; // :020

	virtual void ModifyBeginEx( bool bReadOnly, int firstVertex, int numVerts, int firstIndex, int numIndices, MeshDesc_t_csgo &desc ) = 0; // :021

	virtual void SetFlexMesh( IMesh_csgo *pMesh, int nVertexOffset ) = 0; //:022

	virtual void DisableFlexMesh() = 0; // :023

	virtual void MarkAsDrawn() = 0; // :024 // Not sure if to use it's safe, in some classes this does nothing actually.

	// NOTE: I chose to create this method strictly because it's 2 days to code lock
	// and I could use the DrawInstances technique without a larger code change
	// Draws the mesh w/ modulation.
	virtual void DrawModulated( const Vector4D_csgo &vecDiffuseModulation, int firstIndex = -1, int numIndices = 0 ) = 0; // :025

	virtual unsigned int ComputeMemoryUsed() = 0; // :026

	virtual void *AccessRawHardwareDataStream( uint8 nRawStreamIndex, uint32 numBytes, uint32 uiFlags, void *pvContext ) = 0; // :027

	virtual ICachedPerFrameMeshData_csgo *GetCachedPerFrameMeshData() = 0; // :028
	virtual void ReconstructFromCachedPerFrameMeshData( ICachedPerFrameMeshData_csgo *pData ) = 0; // :029
};

class IMeshEx_csgo abstract : public IMesh_csgo
{
public:
	//
	// Those are probably not required anymore, but better safe than sorry:

	virtual void _UNKNOWN_030(void) = 0;
	virtual void _UNKNOWN_031(void) = 0;
	virtual void _UNKNOWN_032(void) = 0;
	virtual void _UNKNOWN_033(void) = 0;
	virtual void _UNKNOWN_034(void) = 0;
	virtual void _UNKNOWN_035(void) = 0;
	virtual void _UNKNOWN_036(void) = 0;
	virtual void _UNKNOWN_037(void) = 0;
	virtual void _UNKNOWN_038(void) = 0;
	virtual void _UNKNOWN_039(void) = 0;
	virtual void _UNKNOWN_040(void) = 0;
	virtual void _UNKNOWN_041(void) = 0;
	virtual void _UNKNOWN_042(void) = 0;
	virtual void _UNKNOWN_043(void) = 0;
	virtual void _UNKNOWN_044(void) = 0;
	virtual void _UNKNOWN_045(void) = 0;
	virtual void _UNKNOWN_046(void) = 0;
	virtual void _UNKNOWN_047(void) = 0;
	virtual void _UNKNOWN_048(void) = 0;
};

// IMatRenderContext_csgo //////////////////////////////////////////////////////

namespace CSGO {

class ICallQueue;

} // namespace CSGO {

struct MeshInstanceData_t_csgo
{
	int						m_nIndexOffset;
	int						m_nIndexCount;
	int						m_nBoneCount;
	MeshBoneRemap_t_csgo *		m_pBoneRemap;		// there are bone count of these, they index into pose to world
	matrix3x4_t_csgo	*			m_pPoseToWorld;	// transforms for the *entire* model, indexed into by m_pBoneIndex. Potentially more than bone count of these
	const ITexture_csgo *		m_pEnvCubemap;
	MaterialLightingState_t_csgo *m_pLightingState;
	MaterialPrimitiveType_t_csgo m_nPrimType;
	const IVertexBuffer_csgo	*	m_pVertexBuffer;
	int						m_nVertexOffsetInBytes;
	const IIndexBuffer_csgo *	m_pIndexBuffer;
	const IVertexBuffer_csgo	*	m_pColorBuffer;
	int						m_nColorVertexOffsetInBytes;
	ShaderStencilState_t_csgo *	m_pStencilState;
	Vector4D_csgo				m_DiffuseModulation;
	int _UNKNOWN_0x4c;
	void * _UNKNOWN_0x50;
};

enum ImageFormat_csgo 
{ 
	IMAGE_FORMAT_UNKNOWN  = -1,
	IMAGE_FORMAT_RGBA8888 = 0, 
	IMAGE_FORMAT_ABGR8888, 
	IMAGE_FORMAT_RGB888, 
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_RGB565, 
	IMAGE_FORMAT_I8,
	IMAGE_FORMAT_IA88,
	IMAGE_FORMAT_P8,
	IMAGE_FORMAT_A8,
	IMAGE_FORMAT_RGB888_BLUESCREEN,
	IMAGE_FORMAT_BGR888_BLUESCREEN,
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_DXT1,
	IMAGE_FORMAT_DXT3,
	IMAGE_FORMAT_DXT5,
	IMAGE_FORMAT_BGRX8888,
	IMAGE_FORMAT_BGR565,
	IMAGE_FORMAT_BGRX5551,
	IMAGE_FORMAT_BGRA4444,
	IMAGE_FORMAT_DXT1_ONEBITALPHA,
	IMAGE_FORMAT_BGRA5551,
	IMAGE_FORMAT_UV88,
	IMAGE_FORMAT_UVWQ8888,
	IMAGE_FORMAT_RGBA16161616F,
	IMAGE_FORMAT_RGBA16161616,
	IMAGE_FORMAT_UVLX8888,
	IMAGE_FORMAT_R32F,			// Single-channel 32-bit floating point
	IMAGE_FORMAT_RGB323232F,	// NOTE: D3D9 does not have this format
	IMAGE_FORMAT_RGBA32323232F,
	IMAGE_FORMAT_RG1616F,
	IMAGE_FORMAT_RG3232F,
	IMAGE_FORMAT_RGBX8888,

	IMAGE_FORMAT_NULL,			// Dummy format which takes no video memory

	// Compressed normal map formats
	IMAGE_FORMAT_ATI2N,			// One-surface ATI2N / DXN format
	IMAGE_FORMAT_ATI1N,			// Two-surface ATI1N format

	IMAGE_FORMAT_RGBA1010102,	// 10 bit-per component render targets
	IMAGE_FORMAT_BGRA1010102,
	IMAGE_FORMAT_R16F,			// 16 bit FP format

	// Depth-stencil texture formats
	IMAGE_FORMAT_D16,
	IMAGE_FORMAT_D15S1,
	IMAGE_FORMAT_D32,
	IMAGE_FORMAT_D24S8,
	IMAGE_FORMAT_LINEAR_D24S8,
	IMAGE_FORMAT_D24X8,
	IMAGE_FORMAT_D24X4S4,
	IMAGE_FORMAT_D24FS8,
	IMAGE_FORMAT_D16_SHADOW,	// Specific formats for shadow mapping
	IMAGE_FORMAT_D24X8_SHADOW,	// Specific formats for shadow mapping

	// supporting these specific formats as non-tiled for procedural cpu access (360-specific)
	IMAGE_FORMAT_LINEAR_BGRX8888,
	IMAGE_FORMAT_LINEAR_RGBA8888,
	IMAGE_FORMAT_LINEAR_ABGR8888,
	IMAGE_FORMAT_LINEAR_ARGB8888,
	IMAGE_FORMAT_LINEAR_BGRA8888,
	IMAGE_FORMAT_LINEAR_RGB888,
	IMAGE_FORMAT_LINEAR_BGR888,
	IMAGE_FORMAT_LINEAR_BGRX5551,
	IMAGE_FORMAT_LINEAR_I8,
	IMAGE_FORMAT_LINEAR_RGBA16161616,

	IMAGE_FORMAT_LE_BGRX8888,
	IMAGE_FORMAT_LE_BGRA8888,

	NUM_IMAGE_FORMATS
};

class IMatRenderContext_csgo abstract : public IRefCounted_csgo
{
public:
	// 002:
	virtual void BeginRender() = 0;

	// 003:
	virtual void EndRender() = 0;

	// 004;
	virtual void Flush( bool flushHardware = false ) = 0;

	virtual void _UNKNOWN_005(void) = 0;
	virtual void _UNKNOWN_006(void) = 0;
	virtual void _UNKNOWN_007(void) = 0;
	virtual void _UNUSED_008_GetRenderTargetDimensions(void) = 0;

	// 009:
	virtual void Bind( IMaterial_csgo *material, void *proxyData = 0 ) = 0;

	virtual void _UNKNOWN_010(void) = 0;
	virtual void _UNKNOWN_011(void) = 0;
	
	// 012:
	virtual void ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil = false ) = 0;
	
	// 013:
	// read to a unsigned char rgb image. 
	virtual void ReadPixels( int x, int y, int width, int height, unsigned char *data, ImageFormat_csgo dstFormat, unsigned __int32 _unknown7 = 0) = 0; 

	virtual void _UNKNOWN_014(void) = 0;
	virtual void _UNKNOWN_015(void) = 0;
	virtual void _UNKNOWN_016(void) = 0;
	virtual void _UNKNOWN_017(void) = 0;
	virtual void _UNKNOWN_018(void) = 0;
	virtual void _UNKNOWN_019(void) = 0;
	virtual void _UNKNOWN_020(void) = 0;
	virtual void _UNKNOWN_021(void) = 0;
	virtual void _UNKNOWN_022(void) = 0;
	virtual void _UNKNOWN_023(void) = 0;
	virtual void _UNKNOWN_024(void) = 0;
	virtual void _UNKNOWN_025(void) = 0;
	virtual void _UNKNOWN_026(void) = 0;
	virtual void _UNKNOWN_027(void) = 0;
	virtual void _UNKNOWN_028(void) = 0;
	virtual void _UNKNOWN_029(void) = 0;
	virtual void _UNKNOWN_030(void) = 0;
	virtual void _UNKNOWN_031(void) = 0;
	virtual void _UNKNOWN_032(void) = 0;
	virtual void _UNKNOWN_033(void) = 0;
	virtual void _UNKNOWN_034(void) = 0;
	virtual void _UNKNOWN_035(void) = 0;
	virtual void _UNKNOWN_036(void) = 0;
	virtual void _UNKNOWN_037(void) = 0;
	virtual void _UNKNOWN_038(void) = 0;
	virtual void _UNKNOWN_039(void) = 0;
	virtual void _UNKNOWN_040(void) = 0;
	virtual void _UNKNOWN_041(void) = 0;
	virtual void _UNKNOWN_042(void) = 0;
	virtual void _UNKNOWN_043(void) = 0;
	virtual void _UNKNOWN_044(void) = 0;
	virtual void _UNKNOWN_045(void) = 0;
	virtual void _UNKNOWN_046(void) = 0;
	virtual void _UNKNOWN_047(void) = 0;
	virtual void _UNKNOWN_048(void) = 0;
	virtual void _UNKNOWN_049(void) = 0;
	virtual void _UNKNOWN_050(void) = 0;
	virtual void _UNKNOWN_051(void) = 0;
	virtual void _UNKNOWN_052(void) = 0;
	virtual void _UNKNOWN_053(void) = 0;
	virtual void _UNKNOWN_054(void) = 0;
	virtual void _UNKNOWN_055(void) = 0;
	virtual void _UNKNOWN_056(void) = 0;
	virtual void _UNKNOWN_057(void) = 0;
	virtual void _UNKNOWN_058(void) = 0;
	virtual void _UNKNOWN_059(void) = 0;
	virtual void _UNKNOWN_060(void) = 0;
	virtual void _UNKNOWN_061(void) = 0;

	// 062:
	virtual IMeshEx_csgo* GetDynamicMesh( 
		bool buffered = true, 
		IMesh_csgo* pVertexOverride = 0,	
		IMesh_csgo* pIndexOverride = 0, 
		IMaterial_csgo *pAutoBind = 0 ) = 0;

	virtual void _UNKNOWN_063(void) = 0; // CreateStaticVertexBuffer*?
	virtual void _UNKNOWN_064(void) = 0; // CreateStaticIndexBuffer*?
	virtual void _UNKNOWN_065(void) = 0; // DestroyVertexBuffer*?
	virtual void _UNKNOWN_066(void) = 0; // DestroyIndexBuffer*?
	virtual void _UNKNOWN_067(void) = 0; // GetDynamicVertexBuffer*?
	virtual void _UNKNOWN_068(void) = 0; // GetDynamicIndexBuffer?
	virtual void _UNKNOWN_069(void) = 0; // BindVertexBuffer*?
	virtual void _UNKNOWN_070(void) = 0; // BindIndexBuffer?
	virtual void _UNKNOWN_071(void) = 0; // Draw?
	virtual void _UNKNOWN_072(void) = 0; // SelectionMode?
	virtual void _UNKNOWN_073(void) = 0; // SelectionBuffer?
	virtual void _UNKNOWN_074(void) = 0; // ClearSelectionNames?
	virtual void _UNKNOWN_075(void) = 0; // LoadSelectionName?
	virtual void _UNKNOWN_076(void) = 0; // PushSelectionName?
	virtual void _UNKNOWN_077(void) = 0; // PopSelectionName?
	virtual void _UNKNOWN_078(void) = 0; // ClearColor3ub
	
	// 079:
	virtual void ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a ) = 0;

	virtual void _UNKNOWN_080(void) = 0; // OverrideDepthEnable?
	
	// 081:
	virtual void DrawScreenSpaceQuad(IMaterial_csgo * pMaterial) = 0;

	virtual void _UNKNOWN_082(void) = 0;
	virtual void _UNKNOWN_083(void) = 0;
	virtual void _UNKNOWN_084(void) = 0;
	virtual void _UNKNOWN_085(void) = 0;
	virtual void _UNKNOWN_086(void) = 0;
	virtual void _UNKNOWN_087(void) = 0;
	virtual void _UNKNOWN_088(void) = 0;
	virtual void _UNKNOWN_089(void) = 0;
	virtual void _UNKNOWN_090(void) = 0;
	virtual void _UNKNOWN_091(void) = 0;
	virtual void _UNKNOWN_092(void) = 0;
	virtual void _UNKNOWN_093(void) = 0;
	virtual void _UNKNOWN_094(void) = 0;
	virtual void _UNKNOWN_095(void) = 0;
	virtual void _UNKNOWN_096(void) = 0;
	virtual void _UNKNOWN_097(void) = 0;
	virtual void _UNKNOWN_098(void) = 0; // GetFlashlightMode?
	virtual void _UNKNOWN_099(void) = 0; // IsCullingEnabledForSinglePassFlashlight?
	virtual void _UNKNOWN_100(void) = 0; // EnableCullingForSinglePassFlashlight?
	virtual void _UNKNOWN_101(void) = 0;
	virtual void _UNKNOWN_102(void) = 0;
	virtual void _UNKNOWN_103(void) = 0;
	virtual void _UNKNOWN_104(void) = 0;
	virtual void _UNKNOWN_105(void) = 0;
	virtual void _UNKNOWN_106(void) = 0;
	virtual void _UNKNOWN_107(void) = 0;
	virtual void _UNKNOWN_108(void) = 0;
	virtual void _UNKNOWN_109(void) = 0;
	virtual void _UNKNOWN_110(void) = 0;
	virtual void _UNKNOWN_111(void) = 0;
	virtual void _UNKNOWN_112(void) = 0;

	// 113:
	// This function performs a texture map from one texture map to the render destination, doing
	// all the necessary pixel/texel coordinate fix ups. fractional values can be used for the
	// src_texture coordinates to get linear sampling - integer values should produce 1:1 mappings
	// for non-scaled operations.
	virtual void DrawScreenSpaceRectangle( 
		IMaterial_csgo *pMaterial,
		int destx, int desty,
		int width, int height,
		float src_texture_x0, float src_texture_y0,			// which texel you want to appear at
		// destx/y
		float src_texture_x1, float src_texture_y1,			// which texel you want to appear at
		// destx+width-1, desty+height-1
		int src_texture_width, int src_texture_height,		// needed for fixup
		void *pClientRenderable = 0,
		int nXDice = 1,
		int nYDice = 1 )=0;

	virtual void _UNKNOWN_114(void) = 0;
	
	// 118:
	// This version will push the current rendertarget + current viewport onto the stack
	virtual void PushRenderTargetAndViewport( ) = 0;

	// 117:
	// This version will push a new rendertarget + a maximal viewport for that rendertarget onto the stack
	virtual void PushRenderTargetAndViewport( ITexture_csgo *pTexture ) = 0;

	// 116:
	// This version will push a new rendertarget + a specified viewport onto the stack
	virtual void PushRenderTargetAndViewport( ITexture_csgo *pTexture, int nViewX, int nViewY, int nViewW, int nViewH ) = 0;

	// 115:
	// This version will push a new rendertarget + a specified viewport onto the stack
	virtual void PushRenderTargetAndViewport( ITexture_csgo *pTexture, ITexture_csgo *pDepthTexture, int nViewX, int nViewY, int nViewW, int nViewH ) = 0;

	// 119:
	// This will pop a rendertarget + viewport
	virtual void PopRenderTargetAndViewport( void ) = 0;

	virtual void _UNKNOWN_120(void) = 0; // BindLightmapTexture?
	virtual void _UNKNOWN_121(void) = 0; // CopyRenderTargetToTextureEx?
	virtual void _UNKNOWN_122(void) = 0;
	virtual void _UNKNOWN_123(void) = 0;
	virtual void _UNKNOWN_124(void) = 0;
	virtual void _UNKNOWN_125(void) = 0;
	virtual void _UNKNOWN_126(void) = 0;
	virtual void _UNKNOWN_127(void) = 0;
	virtual void _UNKNOWN_128(void) = 0;
	virtual void _UNKNOWN_129(void) = 0; 
	virtual void _UNKNOWN_130(void) = 0; // PushCustomClipPlane?
	virtual void _UNKNOWN_131(void) = 0; // PopCustomClipPlane?
	virtual void _UNKNOWN_132(void) = 0; // GetMaxToRender?
	virtual void _UNKNOWN_133(void) = 0; // GetMaxVerticesToRender?
	virtual void _UNKNOWN_134(void) = 0; // GetMaxIndicesToRender?
	virtual void _UNKNOWN_135(void) = 0;
	virtual void _UNKNOWN_136(void) = 0;
	virtual void _UNKNOWN_137(void) = 0;
	virtual void _UNKNOWN_138(void) = 0;
	virtual void _UNKNOWN_139(void) = 0;
	virtual void _UNKNOWN_140(void) = 0;
	virtual void _UNKNOWN_141(void) = 0;
	virtual void _UNKNOWN_142(void) = 0;
	virtual void _UNKNOWN_143(void) = 0;
	virtual void _UNKNOWN_144(void) = 0;
	virtual void _UNKNOWN_145(void) = 0;
	virtual void _UNKNOWN_146(void) = 0;
	virtual void _UNKNOWN_147(void) = 0;
	virtual void _UNKNOWN_148(void) = 0;
	virtual void _UNKNOWN_149(void) = 0;
	
	// Raw access to the call queue, which can be NULL if not in a queued mode
	virtual SOURCESDK::CSGO::ICallQueue *GetCallQueue() = 0; //:150

	virtual void _UNKNOWN_151(void) = 0;
	virtual void _UNKNOWN_152(void) = 0;
	virtual void _UNKNOWN_153(void) = 0;
	virtual void _UNKNOWN_154(void) = 0;
	virtual void _UNKNOWN_155(void) = 0;
	virtual void _UNKNOWN_156(void) = 0;
	virtual void _UNKNOWN_157(void) = 0;
	virtual void _UNKNOWN_158(void) = 0;
	virtual void _UNKNOWN_159(void) = 0;
	virtual void _UNKNOWN_160(void) = 0;
	virtual void _UNKNOWN_161(void) = 0;
	virtual void _UNKNOWN_162(void) = 0;
	virtual void _UNKNOWN_163(void) = 0;
	virtual void _UNKNOWN_164(void) = 0;
	virtual void _UNKNOWN_165(void) = 0; 
	virtual void _UNKNOWN_166(void) = 0; // GetMorphAccumulatorTexCoord?
	
	// 167:
	virtual IMeshEx_csgo* GetDynamicMeshEx(
		unsigned __int32 _unknown1,
		bool buffered, 
		unsigned __int32 _unknown2,
		IMesh_csgo* pVertexOverride = 0,	
		IMesh_csgo* pIndexOverride = 0, 
		IMaterial_csgo *pAutoBind = 0 ) = 0;

	virtual void _UNKNOWN_168(void) = 0; // FogMaxDensity?
	virtual void _UNKNOWN_169(void) = 0;
	virtual void _UNKNOWN_170(void) = 0;
	virtual void _UNKNOWN_171(void) = 0;
	virtual void _UNKNOWN_172(void) = 0;
	virtual void _UNKNOWN_173(void) = 0;
	virtual void _UNKNOWN_174(void) = 0;
	virtual void _UNKNOWN_175(void) = 0;
	virtual void _UNKNOWN_176(void) = 0;
	virtual void _UNKNOWN_177(void) = 0;
	virtual void _UNKNOWN_178(void) = 0;
	virtual void _UNKNOWN_179(void) = 0;
	virtual void _UNKNOWN_180(void) = 0;
	virtual void _UNKNOWN_181(void) = 0;
	virtual void _UNKNOWN_182(void) = 0;
	virtual void _UNKNOWN_183(void) = 0;
	virtual void _UNKNOWN_184(void) = 0;
	virtual void _UNKNOWN_185(void) = 0;
	virtual void _UNKNOWN_186(void) = 0;
	virtual void _UNKNOWN_187(void) = 0;
	virtual void _UNKNOWN_188(void) = 0;
	virtual void _UNKNOWN_189(void) = 0;
	virtual void _UNKNOWN_190(void) = 0;
	virtual void _UNKNOWN_191(void) = 0;

	// 192:
	virtual void DrawInstances( int nInstanceCount, const MeshInstanceData_t_csgo *pInstance ) = 0;

	virtual void _UNKNOWN_193(void) = 0; // OverrideAlphaWriteEnable?
	virtual void _UNKNOWN_194(void) = 0; // OverrideColorWriteEnable?
	virtual void _UNKNOWN_195(void) = 0;
	virtual void _UNKNOWN_196(void) = 0;
	virtual void _UNKNOWN_197(void) = 0;
	virtual void _UNKNOWN_198(void) = 0;
	virtual void _UNKNOWN_199(void) = 0;
	virtual void _UNKNOWN_200(void) = 0;
	virtual void _UNKNOWN_201(void) = 0;
	virtual void _UNKNOWN_202(void) = 0;
	virtual void _UNKNOWN_203(void) = 0;
	virtual void _UNKNOWN_204(void) = 0;
	virtual void _UNKNOWN_205(void) = 0;
	virtual void _UNKNOWN_206(void) = 0;
	virtual void _UNKNOWN_207(void) = 0;
	virtual void _UNKNOWN_208(void) = 0;
	virtual void _UNKNOWN_209(void) = 0;
	virtual void _UNKNOWN_210(void) = 0;
	virtual void _UNKNOWN_211(void) = 0;
	virtual void _UNKNOWN_212(void) = 0;
	virtual void _UNKNOWN_213(void) = 0;
	virtual void _UNKNOWN_214(void) = 0;
	virtual void _UNKNOWN_215(void) = 0;
	virtual void _UNKNOWN_216(void) = 0;
	virtual void _UNKNOWN_217(void) = 0;
	virtual void _UNKNOWN_218(void) = 0;
	virtual void _UNKNOWN_219(void) = 0;
	virtual void _UNKNOWN_220(void) = 0;
	virtual void _UNKNOWN_221(void) = 0;
	virtual void _UNKNOWN_222(void) = 0;
	virtual void _UNKNOWN_223(void) = 0;
	virtual void _UNKNOWN_224(void) = 0;
	virtual void _UNKNOWN_225(void) = 0;
	virtual void _UNKNOWN_226(void) = 0;
	virtual void _UNKNOWN_227(void) = 0;
	virtual void _UNKNOWN_228(void) = 0;
	virtual void _UNKNOWN_229(void) = 0;
	virtual void _UNKNOWN_230(void) = 0;
	virtual void _UNKNOWN_231(void) = 0;
	virtual void _UNKNOWN_232(void) = 0;
	virtual void _UNKNOWN_233(void) = 0;
	virtual void _UNKNOWN_234(void) = 0;
	virtual void _UNKNOWN_235(void) = 0;
	virtual void _UNKNOWN_236(void) = 0;
	virtual void _UNKNOWN_237(void) = 0;
	virtual void _UNKNOWN_238(void) = 0;
	virtual void _UNKNOWN_239(void) = 0;
	virtual void _UNKNOWN_240(void) = 0;
	virtual void _UNKNOWN_241(void) = 0;
	virtual void _UNKNOWN_242(void) = 0;
	virtual void _UNKNOWN_243(void) = 0;
	virtual void _UNKNOWN_244(void) = 0;
	virtual void _UNKNOWN_245(void) = 0;
	virtual void _UNKNOWN_246(void) = 0;
	virtual void _UNKNOWN_247(void) = 0;
	virtual void _UNKNOWN_248(void) = 0;
	virtual void _UNKNOWN_249(void) = 0;
	virtual void _UNKNOWN_250(void) = 0;
	virtual void _UNKNOWN_251(void) = 0;
	virtual void _UNKNOWN_252(void) = 0;
};

// IMaterialSystem_csgo ////////////////////////////////////////////////////////

#define MATERIAL_SYSTEM_INTERFACE_VERSION_CSGO_80 "VMaterialSystem080"

typedef void * MaterialLock_t_csgo;

// NOTE: All size modes will force the render target to be smaller than or equal to
// the size of the framebuffer.
enum RenderTargetSizeMode_t_csgo
{
	RT_SIZE_NO_CHANGE=0,			// Only allowed for render targets that don't want a depth buffer
	// (because if they have a depth buffer, the render target must be less than or equal to the size of the framebuffer).
	RT_SIZE_DEFAULT=1,				// Don't play with the specified width and height other than making sure it fits in the framebuffer.
	RT_SIZE_PICMIP=2,				// Apply picmip to the render target's width and height.
	RT_SIZE_HDR=3,					// frame_buffer_width / 4
	RT_SIZE_FULL_FRAME_BUFFER=4,	// Same size as frame buffer, or next lower power of 2 if we can't do that.
	RT_SIZE_OFFSCREEN=5,			// Target of specified size, don't mess with dimensions
	RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP=6 // Same size as the frame buffer, rounded up if necessary for systems that can't do non-power of two textures.
};

enum MaterialRenderTargetDepth_t_csgo
{
	MATERIAL_RT_DEPTH_SHARED   = 0x0,
	MATERIAL_RT_DEPTH_SEPARATE = 0x1,
	MATERIAL_RT_DEPTH_NONE     = 0x2,
	MATERIAL_RT_DEPTH_ONLY	   = 0x3,
};

class IMaterialSystem_csgo abstract : public IAppSystem_csgo
{
public:
	virtual void _UNKNOWN_009(void) = 0; // Init
	virtual void _UNKNOWN_010(void) = 0; // SetShaderAPI
	virtual void _UNKNOWN_011(void) = 0; // SetAdapter
	virtual void _UNKNOWN_012(void) = 0; // ModInit
	virtual void _UNKNOWN_013(void) = 0; // ModShutdown
	virtual void _UNKNOWN_014(void) = 0;
	virtual void _UNKNOWN_015(void) = 0;
	virtual void _UNKNOWN_016(void) = 0;
	virtual void _UNKNOWN_017(void) = 0;
	virtual void _UNKNOWN_018(void) = 0;
	virtual void _UNKNOWN_019(void) = 0;
	virtual void _UNKNOWN_020(void) = 0;
	virtual void _UNKNOWN_021(void) = 0;
	virtual void _UNKNOWN_022(void) = 0;
	virtual void _UNKNOWN_023(void) = 0;
	virtual void _UNKNOWN_024(void) = 0;
	virtual void _UNKNOWN_025(void) = 0;
	virtual void _UNKNOWN_026(void) = 0;
	virtual void _UNKNOWN_027(void) = 0;
	virtual void _UNKNOWN_028(void) = 0;
	virtual void _UNKNOWN_029(void) = 0;
	virtual void _UNKNOWN_030(void) = 0;
	virtual void _UNKNOWN_031(void) = 0;
	virtual void _UNKNOWN_032(void) = 0;
	virtual void _UNKNOWN_033(void) = 0;
	virtual void _UNKNOWN_034(void) = 0;
	virtual void _UNKNOWN_035(void) = 0;
	virtual void _UNKNOWN_036(void) = 0;
	virtual void _UNKNOWN_037(void) = 0;
	virtual void _UNKNOWN_038(void) = 0;
	virtual void _UNKNOWN_039(void) = 0;
	virtual void _UNKNOWN_040(void) = 0;
	virtual void _UNKNOWN_041(void) = 0;
	
	virtual void				BeginFrame(float frameTime) = 0; //:042
	virtual void				EndFrame() = 0; //:043

	virtual void _UNKNOWN_044(void) = 0;
	virtual void _UNKNOWN_045(void) = 0;

	// 046:
	virtual void SwapBuffers() = 0;

	virtual void _UNKNOWN_047(void) = 0;
	virtual void _UNKNOWN_048(void) = 0;
	virtual void _UNKNOWN_049(void) = 0;
	virtual void _UNKNOWN_050(void) = 0;
	virtual void _UNKNOWN_051(void) = 0;
	virtual void _UNKNOWN_052(void) = 0;
	virtual void _UNKNOWN_053(void) = 0;
	virtual void _UNKNOWN_054(void) = 0;
	virtual void _UNKNOWN_055(void) = 0;
	virtual void _UNKNOWN_056(void) = 0;
	virtual void _UNKNOWN_057(void) = 0;
	virtual void _UNKNOWN_058(void) = 0;
	virtual void _UNKNOWN_059(void) = 0;
	virtual void _UNKNOWN_060(void) = 0;
	virtual void _UNKNOWN_061(void) = 0;
	virtual void _UNKNOWN_062(void) = 0;
	virtual void _UNKNOWN_063(void) = 0;
	virtual void _UNKNOWN_064(void) = 0;
	virtual void _UNKNOWN_065(void) = 0;
	virtual void _UNKNOWN_066(void) = 0;
	virtual void _UNKNOWN_067(void) = 0;
	virtual void _UNKNOWN_068(void) = 0;
	virtual void _UNKNOWN_069(void) = 0;
	virtual void _UNKNOWN_070(void) = 0;
	virtual void _UNKNOWN_071(void) = 0;
	virtual void _UNKNOWN_072(void) = 0;
	virtual void _UNKNOWN_073(void) = 0;
	virtual void _UNKNOWN_074(void) = 0;
	virtual void _UNKNOWN_075(void) = 0;
	virtual void _UNKNOWN_076(void) = 0;
	virtual void _UNKNOWN_077(void) = 0;
	virtual void _UNKNOWN_078(void) = 0;
	virtual void _UNKNOWN_079(void) = 0;
	virtual void _UNKNOWN_080(void) = 0;
	virtual void _UNKNOWN_081(void) = 0; 
	virtual void _UNKNOWN_082(void) = 0; // ? ReloadMaterials
	virtual void _UNKNOWN_083(void) = 0; // CreateMaterial

	// 084:
	virtual IMaterial_csgo * FindMaterial( char const* pMaterialName, const char *pTextureGroupName, bool complain = true, const char *pComplainPrefix = 0 ) = 0;
	
	virtual void _UNKNOWN_085(void) = 0; // ?
	virtual void _UNKNOWN_086(void) = 0; // FirstMaterial
	virtual void _UNKNOWN_087(void) = 0; // NextMaterial
	virtual void _UNKNOWN_088(void) = 0; // InvalidMaterial
	virtual void _UNKNOWN_089(void) = 0; // GetMaterial
	virtual void _UNKNOWN_090(void) = 0; // GetNumMaterials
	virtual void _UNKNOWN_091(void) = 0; // FindTexture
	virtual void _UNKNOWN_092(void) = 0; // IsTextureLoaded
	virtual void _UNKNOWN_093(void) = 0; // CreateProceduralTexture

	//
	// Render targets
	//

	// 094:
	virtual void BeginRenderTargetAllocation() = 0;

	// 095:
	virtual void EndRenderTargetAllocation() = 0; // Simulate an Alt-Tab in here, which causes a release/restore of all resources

	// 096:
	// Creates a render target
	// If depth == true, a depth buffer is also allocated. If not, then
	// the screen's depth buffer is used.
	// Creates a texture for use as a render target
	virtual ITexture_csgo *	CreateRenderTargetTexture( int w, 
		int h, 
		RenderTargetSizeMode_t_csgo sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		ImageFormat_csgo	format, 
		MaterialRenderTargetDepth_t_csgo depth = MATERIAL_RT_DEPTH_SHARED ) = 0;

	virtual void _UNKNOWN_097(void) = 0;
	virtual void _UNKNOWN_098(void) = 0;
	virtual void _UNKNOWN_099(void) = 0;
	virtual void _UNKNOWN_100(void) = 0;
	virtual void _UNKNOWN_101(void) = 0;
	virtual void _UNKNOWN_102(void) = 0;
	virtual void _UNKNOWN_103(void) = 0;
	virtual void _UNKNOWN_104(void) = 0;
	virtual void _UNKNOWN_105(void) = 0;
	virtual void _UNKNOWN_106(void) = 0;
	virtual void _UNKNOWN_107(void) = 0;
	virtual void _UNKNOWN_108(void) = 0;
	virtual void _UNKNOWN_109(void) = 0;
	virtual void _UNKNOWN_110(void) = 0;
	virtual void _UNKNOWN_111(void) = 0;
	virtual void _UNKNOWN_112(void) = 0;
	virtual void _UNKNOWN_113(void) = 0;
	virtual void _UNKNOWN_114(void) = 0;
	
	// 115:
	/// <remarks>GetRenderContext calls AddRef on returned!</remarks>
	virtual IMatRenderContext_csgo *	GetRenderContext() = 0;

	virtual void _UNKNOWN_116(void) = 0;
	virtual void _UNKNOWN_117(void) = 0;
	virtual void _UNKNOWN_118(void) = 0;
	virtual void _UNKNOWN_119(void) = 0;
	virtual void _UNKNOWN_120(void) = 0;

	// 121:
	/// <remarks>SetRenderContext calls AddRef on param!</remarks>
	virtual IMatRenderContext_csgo *SetRenderContext( IMatRenderContext_csgo * ) = 0;

	// ...
	// more that we don't care about
};

// IBaseClientDLL_csgo /////////////////////////////////////////////////////////

class CViewSetup_csgo;

struct vrect_t_csgo
{
	int				x,y,width,height;
	vrect_t_csgo			*pnext;
};

enum ClearFlags_t_csgo
{ 
	VIEW_CLEAR_COLOR = 0x1,
	VIEW_CLEAR_DEPTH = 0x2,
	VIEW_CLEAR_FULL_TARGET = 0x4,
	VIEW_NO_DRAW = 0x8,
	VIEW_CLEAR_OBEY_STENCIL = 0x10, // Draws a quad allowing stencil test to clear through portals
	VIEW_CLEAR_STENCIL = 0x20,
};

// Used by RenderView 
enum RenderViewInfo_t_csgo
{ 
	RENDERVIEW_UNSPECIFIED	 = 0,
	RENDERVIEW_DRAWVIEWMODEL = (1<<0), 
	RENDERVIEW_DRAWHUD		 = (1<<1),
	RENDERVIEW_SUPPRESSMONITORRENDERING = (1<<2),
};

#define CLIENT_DLL_INTERFACE_VERSION_CSGO_018 "VClient018"

namespace CSGO
{
//-----------------------------------------------------------------------------
// Purpose: The engine reports to the client DLL what stage it's entering so the DLL can latch events
//  and make sure that certain operations only happen during the right stages.
// The value for each stage goes up as you move through the frame so you can check ranges of values
//  and if new stages get added in-between, the range is still valid.
//-----------------------------------------------------------------------------
enum ClientFrameStage_t
{
	FRAME_UNDEFINED = -1,			// (haven't run any frames yet)
	FRAME_START,

	// A network packet is being recieved
	FRAME_NET_UPDATE_START,
	// Data has been received and we're going to start calling PostDataUpdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// Data has been received and we've called PostDataUpdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_NET_UPDATE_END,

	// We're about to start rendering the scene
	FRAME_RENDER_START,
	// We've finished rendering the scene.
	FRAME_RENDER_END
};
}

class IBaseClientDLL_csgo abstract
{
public:
	// 000:
	// Connect appsystem components, get global interfaces, don't run any other init code
	virtual int Connect( CreateInterfaceFn appSystemFactory, CGlobalVarsBase *pGlobals ) = 0;

	// 001:
	virtual void Disconnect() = 0;

	// 002:
	// run other init code here
	virtual int Init( CreateInterfaceFn appSystemFactory, CGlobalVarsBase *pGlobals ) = 0;

	// 003:
	virtual void PostInit() = 0;

	// 004:
	// Called once when the client DLL is being unloaded
	virtual void Shutdown( void ) = 0;

	// 005:
	// Called at the start of each level change
	virtual void LevelInitPreEntity( char const* pMapName ) = 0;

	// 006:
	// Called at the start of a new level, after the entities have been received and created
	virtual void LevelInitPostEntity( ) = 0;

	// 007:
	// Called at the end of a level
	virtual void LevelShutdown( void ) = 0;
	
	virtual void _UNKOWN_008(void) = 0;
	virtual void _UNKOWN_009(void) = 0;
	virtual void _UNKOWN_010(void) = 0;
	virtual void _UNKOWN_011(void) = 0;
	virtual void _UNKOWN_012(void) = 0;
	virtual void _UNKOWN_013(void) = 0;
	virtual void _UNKOWN_014(void) = 0;
	virtual void _UNKOWN_015(void) = 0;
	virtual void _UNKOWN_016(void) = 0;
	virtual void _UNKOWN_017(void) = 0;
	virtual void _UNKOWN_018(void) = 0;
	virtual void _UNKOWN_019(void) = 0;
	virtual void _UNKOWN_020(void) = 0;
	virtual void _UNKOWN_021(void) = 0;
	virtual void _UNKOWN_022(void) = 0;
	virtual void _UNKOWN_023(void) = 0;
	virtual void _UNKOWN_024(void) = 0;
	virtual void _UNKOWN_025(void) = 0;

	// 026:
	virtual void View_Render( vrect_t_csgo *rect )= 0;

	// 027:
	virtual void RenderView( const CViewSetup_csgo &view, int nClearFlags, int whatToDraw )= 0;

	virtual void _UNKOWN_028(void)= 0;
	virtual void _UNKOWN_029(void) = 0;
	virtual void _UNKOWN_030(void) = 0;
	virtual void _UNKOWN_031(void) = 0;
	virtual void _UNKOWN_032(void) = 0;
	virtual void _UNKOWN_033(void) = 0;
	virtual void _UNKOWN_034(void) = 0;
	virtual void _UNKOWN_035(void) = 0;
	
	// 036:
	virtual void FrameStageNotify(SOURCESDK::CSGO::ClientFrameStage_t curStage) = 0;

	virtual void _UNKOWN_037(void) = 0;
	virtual void _UNKOWN_038(void) = 0;
	virtual void _UNKOWN_039(void) = 0;
	virtual void _UNKOWN_040(void) = 0;
	virtual void _UNKOWN_041(void) = 0;
	virtual void _UNKOWN_042(void) = 0;
	virtual void _UNKOWN_043(void) = 0;
	virtual void _UNKOWN_044(void) = 0;
	virtual void _UNKOWN_045(void) = 0;
	virtual void _UNKOWN_046(void) = 0;
	virtual void _UNKOWN_047(void) = 0;
	virtual void _UNKOWN_048(void) = 0;
	virtual void _UNKOWN_049(void) = 0;
	virtual void _UNKOWN_050(void) = 0;
	virtual void _UNKOWN_051(void) = 0;
	virtual void _UNKOWN_052(void) = 0;
	virtual void _UNKOWN_053(void) = 0;
	virtual void _UNKOWN_054(void) = 0;
	virtual void _UNKOWN_055(void) = 0;
	virtual void _UNKOWN_056(void) = 0;
	virtual void _UNKOWN_057(void) = 0;
	virtual void _UNKOWN_058(void) = 0;
	virtual void _UNKOWN_059(void) = 0;
	virtual void _UNKOWN_060(void) = 0;
	virtual void _UNKOWN_061(void) = 0;
	virtual void _UNKOWN_062(void) = 0;

	// 063:
	virtual void _UNUSED_GetScreenWidth(void) = 0;
	
	// 064:
	virtual void _UNUSED_GetScreenHeight(void) = 0;
	
	// 065:
	virtual void WriteSaveGameScreenshotOfSize( const char *pFilename, int width, int height, bool bCreatePowerOf2Padded = false, bool bWriteVTF = false ) = 0;
	
	virtual void _UNKOWN_066(void) = 0;
	virtual void _UNKOWN_067(void) = 0;
	virtual void _UNKOWN_068(void) = 0;
	virtual void _UNKOWN_069(void) = 0;
	virtual void _UNKOWN_070(void) = 0;
	virtual void _UNKOWN_071(void) = 0;
	virtual void _UNKOWN_072(void) = 0;
	virtual void _UNKOWN_073(void) = 0;
	virtual void _UNKOWN_074(void) = 0;
	virtual void _UNKOWN_075(void) = 0;
	virtual void _UNKOWN_076(void) = 0;
	virtual void _UNKOWN_077(void) = 0;
	virtual void _UNKOWN_078(void) = 0;
	virtual void _UNKOWN_079(void) = 0;
	virtual void _UNKOWN_080(void) = 0;
	virtual void _UNKOWN_081(void) = 0;
	virtual void _UNKOWN_082(void) = 0;
	virtual void _UNKOWN_083(void) = 0;
	virtual void _UNKOWN_084(void) = 0;
	virtual void _UNKOWN_085(void) = 0;
	virtual void _UNKOWN_086(void) = 0;
	virtual void _UNKOWN_087(void) = 0;
	virtual void _UNKOWN_088(void) = 0;
	virtual void _UNKOWN_089(void) = 0;
	virtual void _UNKOWN_090(void) = 0;
	virtual void _UNKOWN_091(void) = 0;
	virtual void _UNKOWN_092(void) = 0;
	virtual void _UNKOWN_093(void) = 0;
	virtual void _UNKOWN_094(void) = 0;
	virtual void _UNKOWN_095(void) = 0;
	virtual void _UNKOWN_096(void) = 0;
	virtual void _UNKOWN_097(void) = 0;
	virtual void _UNKOWN_098(void) = 0;
	virtual void _UNKOWN_099(void) = 0;
	virtual void _UNKOWN_100(void) = 0;
	virtual void _UNKOWN_101(void) = 0;
	virtual void _UNKOWN_102(void) = 0;
	virtual void _UNKOWN_103(void) = 0;
	virtual void _UNKOWN_104(void) = 0;
	virtual void _UNKOWN_105(void) = 0;
	virtual void _UNKOWN_106(void) = 0;
	virtual void _UNKOWN_107(void) = 0;
	virtual void _UNKOWN_108(void) = 0;
	virtual void _UNKOWN_109(void) = 0;
	virtual void _UNKOWN_110(void) = 0;
	virtual void _UNKOWN_111(void) = 0;

	// and a few more to be save from updates:

	virtual void _UNKOWN_112(void) = 0;
	virtual void _UNKOWN_113(void) = 0;
	virtual void _UNKOWN_114(void) = 0;
	virtual void _UNKOWN_115(void) = 0;
	virtual void _UNKOWN_116(void) = 0;
	virtual void _UNKOWN_117(void) = 0;
	virtual void _UNKOWN_118(void) = 0;
	virtual void _UNKOWN_119(void) = 0;
	virtual void _UNKOWN_120(void) = 0;
	virtual void _UNKOWN_121(void) = 0;
	virtual void _UNKOWN_122(void) = 0;
	virtual void _UNKOWN_123(void) = 0;
	virtual void _UNKOWN_124(void) = 0;
	virtual void _UNKOWN_125(void) = 0;
	virtual void _UNKOWN_126(void) = 0;
	virtual void _UNKOWN_127(void) = 0;
	virtual void _UNKOWN_128(void) = 0;
	virtual void _UNKOWN_129(void) = 0;
	virtual void _UNKOWN_130(void) = 0;
};

// IVRenderView_csgo ///////////////////////////////////////////////////////////

#define VENGINE_RENDERVIEW_INTERFACE_VERSION_CSGO "VEngineRenderView014"

class IVRenderView_csgo abstract
{
public:
	virtual void _UNKOWN_000(void) = 0;
	virtual void _UNKOWN_001(void) = 0;
	virtual void _UNKOWN_002(void) = 0;
	virtual void _UNKOWN_003(void) = 0;
	
	// 004:
	virtual void SetBlend( float blend ) = 0;

	// 005
	virtual float GetBlend( void ) = 0;

	// 006:
	virtual void SetColorModulation( float const* blend ) = 0;

	// 007:
	virtual void GetColorModulation( float* blend ) = 0;

	virtual void _UNKOWN_008(void) = 0;
	virtual void _UNKOWN_009(void) = 0;
	virtual void _UNKOWN_010(void) = 0;
	virtual void _UNKOWN_011(void) = 0;
	virtual void _UNKOWN_012(void) = 0;
	virtual void _UNKOWN_013(void) = 0;
	virtual void _UNKOWN_014(void) = 0;
	virtual void _UNKOWN_015(void) = 0;
	virtual void _UNKOWN_016(void) = 0;
	virtual void _UNKOWN_017(void) = 0;
	virtual void _UNKOWN_018(void) = 0;
	virtual void _UNKOWN_019(void) = 0;
	virtual void _UNKOWN_020(void) = 0;
	virtual void _UNKOWN_021(void) = 0;
	virtual void _UNKOWN_022(void) = 0;
	virtual void _UNKOWN_023(void) = 0;
	virtual void _UNKOWN_024(void) = 0;
	virtual void _UNKOWN_025(void) = 0;
	virtual void _UNKOWN_026(void) = 0;
	virtual void _UNKOWN_027(void) = 0;
	virtual void _UNKOWN_028(void) = 0;
	virtual void _UNKOWN_029(void) = 0;
	virtual void _UNKOWN_030(void) = 0;
	virtual void _UNKOWN_031(void) = 0;
	virtual void _UNKOWN_032(void) = 0;
	virtual void _UNKOWN_033(void) = 0;
	virtual void _UNKOWN_034(void) = 0;
	virtual void _UNKOWN_035(void) = 0;
	virtual void _UNKOWN_036(void) = 0;
	virtual void _UNKOWN_037(void) = 0;
	virtual void _UNKOWN_038(void) = 0;
	virtual void _UNKOWN_039(void) = 0;
	virtual void _UNKOWN_040(void) = 0;
	virtual void _UNKOWN_041(void) = 0;
	virtual void _UNKOWN_042(void) = 0;
	virtual void _UNKOWN_043(void) = 0;
	virtual void _UNKOWN_044(void) = 0;
	virtual void _UNKOWN_045(void) = 0;
	virtual void _UNKOWN_046(void) = 0;
	virtual void _UNKOWN_047(void) = 0;
	virtual void _UNKOWN_048(void) = 0;
	virtual void _UNKOWN_049(void) = 0;
	virtual void _UNKOWN_050(void) = 0;
	virtual void _UNKOWN_051(void) = 0;
	virtual void _UNKOWN_052(void) = 0;
	virtual void _UNKOWN_053(void) = 0;
	virtual void _UNKOWN_054(void) = 0;
	virtual void _UNKOWN_055(void) = 0;
	virtual void _UNKOWN_056(void) = 0;
	virtual void _UNKOWN_057(void) = 0;
	virtual void _UNKOWN_058(void) = 0;
	virtual void _UNKOWN_059(void) = 0;
};

// IViewRender_csgo ////////////////////////////////////////////////////////////

class C_BaseEntity_csgo;

class IViewRender_csgo abstract
{
public:
	virtual void _UNKOWN_000(void) = 0;
	virtual void _UNKOWN_001(void) = 0;
	virtual void _UNKOWN_002(void) = 0;
	virtual void _UNKOWN_003(void) = 0;
	virtual void _UNKOWN_004(void) = 0;
	virtual void _UNKOWN_005(void) = 0;
	
	// 006:
	// Called to render just a particular setup ( for timerefresh and envmap creation )
	// First argument is 3d view setup, second is for the HUD (in most cases these are ==, but in split screen the client .dll handles this differently)
	virtual void RenderView( const CViewSetup_csgo &view, const CViewSetup_csgo &hudViewSetup, int nClearFlags, int whatToDraw ) = 0;

	virtual void _UNKOWN_007(void) = 0;
	virtual void _UNKOWN_008(void) = 0;
	virtual void _UNKOWN_009(void) = 0;
	virtual void _UNKOWN_010(void) = 0;
	virtual void _UNKOWN_011(void) = 0;

	// 012:
	virtual const CViewSetup_csgo *GetPlayerViewSetup( int nSlot = -1 ) const = 0;

	// 013:
	virtual const CViewSetup_csgo *GetViewSetup( void ) const = 0;

	virtual void _UNKOWN_014(void) = 0; // DisableVis 
	virtual void _UNKOWN_015(void) = 0;
	virtual void _UNKOWN_016(void) = 0;
	virtual void _UNKOWN_017(void) = 0;
	virtual void _UNKOWN_018(void) = 0;
	virtual void _UNKOWN_019(void) = 0;
	virtual void _UNKOWN_020(void) = 0;
	virtual void _UNKOWN_021(void) = 0;
	virtual void _UNKOWN_022(void) = 0;
	virtual void _UNKOWN_023(void) = 0;
	virtual void _UNKOWN_024(void) = 0;
	virtual void _UNKOWN_025(void) = 0;
	virtual void _UNKOWN_026(void) = 0;
	virtual void _UNKOWN_027(void) = 0;

	virtual C_BaseEntity_csgo *GetCurrentlyDrawingEntity() = 0; //:028

	virtual void		SetCurrentlyDrawingEntity(C_BaseEntity_csgo *pEnt) = 0; //:029

	// 030:
	virtual bool UpdateShadowDepthTexture( ITexture_csgo *pRenderTarget, ITexture_csgo *pDepthTexture, const CViewSetup_csgo &shadowView, unsigned char _unkArg4 = 1, unsigned char _unkArg5 = 0 ) = 0;

	// ...
	// more we don't care about.
};


// IFileSystem_csgo ////////////////////////////////////////////////////////////

enum FilesystemMountRetval_t_csgo
{
	FILESYSTEM_MOUNT_OK = 0,
	FILESYSTEM_MOUNT_FAILED,
};

enum SearchPathAdd_t_csgo
{
	PATH_ADD_TO_HEAD,			// First path searched
	PATH_ADD_TO_TAIL,			// Last path searched
	PATH_ADD_TO_TAIL_ATINDEX,	// First path searched
};

class IBaseFileSystem_csgo abstract
{
public:
	void _UNKNOWN_IBaseFileSystem_csgo_Dummy(); // just to make sure that we get a vtable ptr reserved for this class.

	// ...
	// more we don't care about.
};

#define FILESYSTEM_INTERFACE_VERSION_CSGO_017 "VFileSystem017"

class IFileSystem_csgo abstract : public IAppSystem_csgo, public IBaseFileSystem_csgo
{
public:
	//--------------------------------------------------------
	// Steam operations
	//--------------------------------------------------------

	virtual bool			IsSteam() const = 0;

	// Supplying an extra app id will mount this app in addition 
	// to the one specified in the environment variable "steamappid"
	// 
	// If nExtraAppId is < -1, then it will mount that app ID only.
	// (Was needed by the dedicated server b/c the "SteamAppId" env var only gets passed to steam.dll
	// at load time, so the dedicated couldn't pass it in that way).
	virtual	FilesystemMountRetval_t_csgo MountSteamContent( int nExtraAppId = -1 ) = 0;

	//--------------------------------------------------------
	// Search path manipulation
	//--------------------------------------------------------

	// Add paths in priority order (mod dir, game dir, ....)
	// If one or more .pak files are in the specified directory, then they are
	//  added after the file system path
	// If the path is the relative path to a .bsp file, then any previous .bsp file 
	//  override is cleared and the current .bsp is searched for an embedded PAK file
	//  and this file becomes the highest priority search path ( i.e., it's looked at first
	//   even before the mod's file system path ).
	virtual void			AddSearchPath( const char *pPath, const char *pathID, SearchPathAdd_t_csgo addType = PATH_ADD_TO_TAIL ) = 0;
	virtual bool			RemoveSearchPath( const char *pPath, const char *pathID = 0 ) = 0;

	// Remove all search paths (including write path?)
	virtual void			RemoveAllSearchPaths( void ) = 0;

	// Remove search paths associated with a given pathID
	virtual void			RemoveSearchPaths( const char *szPathID ) = 0;

	// This is for optimization. If you mark a path ID as "by request only", then files inside it
	// will only be accessed if the path ID is specifically requested. Otherwise, it will be ignored.
	// If there are currently no search paths with the specified path ID, then it will still
	// remember it in case you add search paths with this path ID.
	virtual void			MarkPathIDByRequestOnly( const char *pPathID, bool bRequestOnly ) = 0;

	// ...
	// more we don't care about.
};

// CViewSetup_csgo /////////////////////////////////////////////////////////////

class CViewSetup_csgo
{
public:

// shared by 2D & 3D views

	// left side of view window
	int			x;	 
	int			m_nUnscaledX;
	// top side of view window
	int			y;
	int			m_nUnscaledY;
	// width of view window
	int			width;
	int			m_nUnscaledWidth;
	// height of view window
	int			height;
	int			m_nUnscaledHeight;

	char _unknown_20_14c[0x12c];

	bool		m_bCacheFullSceneState : 1;

	// ...
	// more we don't care about.
};

////////////////////////////////////////////////////////////////////////////////

enum ShaderBlendFactor_t_csgo
{
	SHADER_BLEND_ZERO,
	SHADER_BLEND_ONE,
	SHADER_BLEND_DST_COLOR,
	SHADER_BLEND_ONE_MINUS_DST_COLOR,
	SHADER_BLEND_SRC_ALPHA,
	SHADER_BLEND_ONE_MINUS_SRC_ALPHA,
	SHADER_BLEND_DST_ALPHA,
	SHADER_BLEND_ONE_MINUS_DST_ALPHA,
	SHADER_BLEND_SRC_ALPHA_SATURATE,
	SHADER_BLEND_SRC_COLOR,
	SHADER_BLEND_ONE_MINUS_SRC_COLOR
};

// IShaderShadow_csgo //////////////////////////////////////////////////////////

#define SHADERSHADOW_INTERFACE_VERSION_CSGO "ShaderShadow010"

class IShaderShadow_csgo
{
public:
	virtual void _UNKOWN_000(void) = 0;
	virtual void _UNKOWN_001(void) = 0;
	
	// 002:
	virtual void EnableDepthWrites( bool bEnable ) = 0;
	
	virtual void _UNKOWN_003(void) = 0;
	virtual void _UNKOWN_004(void) = 0;

	// 005:
	virtual void EnableColorWrites( bool bEnable ) = 0;

	// 006:
	virtual void EnableAlphaWrites( bool bEnable ) = 0;

	// 007:
	virtual void EnableBlending( bool bEnable ) = 0;

	virtual void _UNKOWN_008(void) = 0;
	
	// 009:
	virtual void BlendFunc( ShaderBlendFactor_t_csgo srcFactor, ShaderBlendFactor_t_csgo dstFactor ) = 0;

	virtual void _UNKOWN_010(void) = 0;
	virtual void _UNKOWN_011(void) = 0;
	virtual void _UNKOWN_012(void) = 0;
	virtual void _UNKOWN_013(void) = 0;
	virtual void _UNKOWN_014(void) = 0;
	virtual void _UNKOWN_015(void) = 0;
	virtual void _UNKOWN_016(void) = 0; // VertexShaderVertexFormat

	// 017:
	virtual void SetVertexShader( const char* pFileName, int nStaticVshIndex ) = 0;

	// 018:
	virtual	void SetPixelShader( const char* pFileName, int nStaticPshIndex = 0 ) = 0;

	// 019:
	// Convert from linear to gamma color space on writes to frame buffer.
	virtual void EnableSRGBWrite( bool bEnable ) = 0;

	virtual void _UNKOWN_020(void) = 0; // EnableSRGBRead, but unimplemented.
	virtual void _UNKOWN_021(void) = 0; // EnableTexture

	virtual void _UNKOWN_022(void) = 0;
	virtual void _UNKOWN_023(void) = 0;
	virtual void _UNKOWN_024(void) = 0;
	virtual void _UNKOWN_025(void) = 0;
	virtual void _UNKOWN_026(void) = 0;
	virtual void _UNKOWN_027(void) = 0;
	virtual void _UNKOWN_028(void) = 0;
	virtual void _UNKOWN_029(void) = 0;
	virtual void _UNKOWN_030(void) = 0;
	virtual void _UNKOWN_031(void) = 0;
	virtual void _UNKOWN_032(void) = 0;
	virtual void _UNKOWN_033(void) = 0;
};


// IShaderDynamicAPI_csgo //////////////////////////////////////////////////////

class IShaderDynamicAPI_csgo
{
public:
	virtual void _UNKOWN_000(void) = 0;
	virtual void _UNKOWN_001(void) = 0;
	virtual void _UNKOWN_002(void) = 0;
	virtual void _UNKOWN_003(void) = 0;
	virtual void _UNKOWN_004(void) = 0;
	virtual void _UNKOWN_005(void) = 0;
	virtual void _UNKOWN_006(void) = 0;
	virtual void _UNKOWN_007(void) = 0;
	virtual void _UNKOWN_008(void) = 0;
	virtual void _UNKOWN_009(void) = 0;
	virtual void _UNKOWN_010(void) = 0;
	virtual void _UNKOWN_011(void) = 0;
	virtual void _UNKOWN_012(void) = 0;
	virtual void _UNKOWN_013(void) = 0;
	virtual void _UNKOWN_014(void) = 0;
	virtual void _UNKOWN_015(void) = 0;
	virtual void _UNKOWN_016(void) = 0; 
	virtual void _UNKOWN_017(void) = 0; 
	virtual void _UNKOWN_018(void) = 0;

	// 019:
	virtual void SetVertexShaderIndex( int vshIndex = -1 ) = 0;

	// 020:
	virtual void SetPixelShaderIndex( int pshIndex = 0 ) = 0;

	virtual void _UNKOWN_021(void) = 0;
	virtual void _UNKOWN_022(void) = 0;
	virtual void _UNKOWN_023(void) = 0;
	virtual void _UNKOWN_024(void) = 0;
	virtual void _UNKOWN_025(void) = 0;
	virtual void _UNKOWN_026(void) = 0;
	virtual void _UNKOWN_027(void) = 0;
	virtual void _UNKOWN_028(void) = 0;
	virtual void _UNKOWN_029(void) = 0;
	virtual void _UNKOWN_030(void) = 0;
	virtual void _UNKOWN_031(void) = 0;
	virtual void _UNKOWN_032(void) = 0;
	virtual void _UNKOWN_033(void) = 0;
	virtual void _UNKOWN_034(void) = 0;
	virtual void _UNKOWN_035(void) = 0;
	virtual void _UNKOWN_036(void) = 0;
	virtual void _UNKOWN_037(void) = 0;
	virtual void _UNKOWN_038(void) = 0;
	virtual void _UNKOWN_039(void) = 0;
	virtual void _UNKOWN_040(void) = 0;
	virtual void _UNKOWN_041(void) = 0;
	virtual void _UNKOWN_042(void) = 0;
	virtual void _UNKOWN_043(void) = 0;
	virtual void _UNKOWN_044(void) = 0;
	virtual void _UNKOWN_045(void) = 0;
	virtual void _UNKOWN_046(void) = 0;
	virtual void _UNKOWN_047(void) = 0;
	virtual void _UNKOWN_048(void) = 0;
	virtual void _UNKOWN_049(void) = 0;
	virtual void _UNKOWN_050(void) = 0;
	virtual void _UNKOWN_051(void) = 0;
	virtual void _UNKOWN_052(void) = 0;
	virtual void _UNKOWN_053(void) = 0;
	virtual void _UNKOWN_054(void) = 0;
	virtual void _UNKOWN_055(void) = 0;
	virtual void _UNKOWN_056(void) = 0;
	virtual void _UNKOWN_057(void) = 0;

	// 058:
	virtual void ExecuteCommandBuffer( uint8 *pCmdBuffer ) = 0;

	virtual void _UNKOWN_059(void) = 0;
	virtual void _UNKOWN_060(void) = 0;
	virtual void _UNKOWN_061(void) = 0;
	virtual void _UNKOWN_062(void) = 0;
	virtual void _UNKOWN_063(void) = 0;
	virtual void _UNKOWN_064(void) = 0;
	virtual void _UNKOWN_065(void) = 0;
	virtual void _UNKOWN_066(void) = 0;
	virtual void _UNKOWN_067(void) = 0;
	virtual void _UNKOWN_068(void) = 0;
	virtual void _UNKOWN_069(void) = 0;
	virtual void _UNKOWN_070(void) = 0;
	virtual void _UNKOWN_071(void) = 0;
	virtual void _UNKOWN_072(void) = 0;
	virtual void _UNKOWN_073(void) = 0;
	virtual void _UNKOWN_074(void) = 0;
	virtual void _UNKOWN_075(void) = 0;
	virtual void _UNKOWN_076(void) = 0;
	virtual void _UNKOWN_077(void) = 0;
	virtual void _UNKOWN_078(void) = 0;
	virtual void _UNKOWN_079(void) = 0;
	virtual void _UNKOWN_080(void) = 0;
	virtual void _UNKOWN_081(void) = 0;
	virtual void _UNKOWN_082(void) = 0;
	virtual void _UNKOWN_083(void) = 0;
	virtual void _UNKOWN_084(void) = 0;
	virtual void _UNKOWN_085(void) = 0;
	virtual void _UNKOWN_086(void) = 0;
	virtual void _UNKOWN_087(void) = 0;
	virtual void _UNKOWN_088(void) = 0;
	virtual void _UNKOWN_089(void) = 0;
	virtual void _UNKOWN_090(void) = 0;
	virtual void _UNKOWN_091(void) = 0;
	virtual void _UNKOWN_092(void) = 0;
	virtual void _UNKOWN_093(void) = 0;
	virtual void _UNKOWN_094(void) = 0;
	virtual void _UNKOWN_095(void) = 0;
	virtual void _UNKOWN_096(void) = 0;
	virtual void _UNKOWN_097(void) = 0;
	virtual void _UNKOWN_098(void) = 0;
	virtual void _UNKOWN_099(void) = 0;
	virtual void _UNKOWN_100(void) = 0;
	virtual void _UNKOWN_101(void) = 0;
	virtual void _UNKOWN_102(void) = 0;
	virtual void _UNKOWN_103(void) = 0;
	virtual void _UNKOWN_104(void) = 0;
	virtual void _UNKOWN_105(void) = 0;
	virtual void _UNKOWN_106(void) = 0;
	virtual void _UNKOWN_107(void) = 0;
	virtual void _UNKOWN_108(void) = 0;
	virtual void _UNKOWN_109(void) = 0;
	virtual void _UNKOWN_110(void) = 0;
	virtual void _UNKOWN_111(void) = 0;
	virtual void _UNKOWN_112(void) = 0;
	virtual void _UNKOWN_113(void) = 0;
	virtual void _UNKOWN_114(void) = 0;
	virtual void _UNKOWN_115(void) = 0;
	virtual void _UNKOWN_116(void) = 0;
	virtual void _UNKOWN_117(void) = 0;
	virtual void _UNKOWN_118(void) = 0;
	virtual void _UNKOWN_119(void) = 0;
	virtual void _UNKOWN_120(void) = 0;
	virtual void _UNKOWN_121(void) = 0;
	virtual void _UNKOWN_122(void) = 0;
	virtual void _UNKOWN_123(void) = 0;
	virtual void _UNKOWN_124(void) = 0;
	virtual void _UNKOWN_125(void) = 0;
	virtual void _UNKOWN_126(void) = 0;
	virtual void _UNKOWN_127(void) = 0;
	virtual void _UNKOWN_128(void) = 0;
	virtual void _UNKOWN_129(void) = 0;
	virtual void _UNKOWN_130(void) = 0;
	virtual void _UNKOWN_131(void) = 0;
	virtual void _UNKOWN_132(void) = 0;
	virtual void _UNKOWN_133(void) = 0;
	virtual void _UNKOWN_134(void) = 0;
	virtual void _UNKOWN_135(void) = 0;
	virtual void _UNKOWN_136(void) = 0;
	virtual void _UNKOWN_137(void) = 0;
	virtual void _UNKOWN_138(void) = 0;
	virtual void _UNKOWN_139(void) = 0;
	virtual void _UNKOWN_140(void) = 0;
	virtual void _UNKOWN_141(void) = 0;
	virtual void _UNKOWN_142(void) = 0;
	virtual void _UNKOWN_143(void) = 0;
	virtual void _UNKOWN_144(void) = 0;
	virtual void _UNKOWN_145(void) = 0;
	virtual void _UNKOWN_146(void) = 0;
};

// IMaterialVar_csgo ///////////////////////////////////////////////////////////

enum MaterialVarSym_t_csgo
{
};

enum ShaderMaterialVars_t_csgo
{
	FLAGS = 0,
	FLAGS_DEFINED,	// mask indicating if the flag was specified
	FLAGS2,
	FLAGS_DEFINED2,
	COLOR,
	ALPHA,
	BASETEXTURE,
	FRAME,
	BASETEXTURETRANSFORM,
	FLASHLIGHTTEXTURE,
	FLASHLIGHTTEXTUREFRAME,
	COLOR2,
	SRGBTINT,

	NUM_SHADER_MATERIAL_VARS
};

enum MaterialVarFlags_t_csgo
{
	MATERIAL_VAR_DEBUG					  = (1 << 0),
	MATERIAL_VAR_NO_DEBUG_OVERRIDE		  = (1 << 1),
	MATERIAL_VAR_NO_DRAW				  = (1 << 2),
	MATERIAL_VAR_USE_IN_FILLRATE_MODE	  = (1 << 3),

	MATERIAL_VAR_VERTEXCOLOR			  = (1 << 4),
	MATERIAL_VAR_VERTEXALPHA			  = (1 << 5),
	MATERIAL_VAR_SELFILLUM				  = (1 << 6),
	MATERIAL_VAR_ADDITIVE				  = (1 << 7),
	MATERIAL_VAR_ALPHATEST				  = (1 << 8),
//	MATERIAL_VAR_UNUSED					  = (1 << 9),
	MATERIAL_VAR_ZNEARER				  = (1 << 10),
	MATERIAL_VAR_MODEL					  = (1 << 11),
	MATERIAL_VAR_FLAT					  = (1 << 12),
	MATERIAL_VAR_NOCULL					  = (1 << 13),
	MATERIAL_VAR_NOFOG					  = (1 << 14),
	MATERIAL_VAR_IGNOREZ				  = (1 << 15),
	MATERIAL_VAR_DECAL					  = (1 << 16),
	MATERIAL_VAR_ENVMAPSPHERE			  = (1 << 17), // OBSOLETE
//	MATERIAL_VAR_UNUSED					  = (1 << 18),
	MATERIAL_VAR_ENVMAPCAMERASPACE	      = (1 << 19), // OBSOLETE
	MATERIAL_VAR_BASEALPHAENVMAPMASK	  = (1 << 20),
	MATERIAL_VAR_TRANSLUCENT              = (1 << 21),
	MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
	MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING  = (1 << 23), // OBSOLETE
	MATERIAL_VAR_OPAQUETEXTURE			  = (1 << 24),
	MATERIAL_VAR_ENVMAPMODE				  = (1 << 25), // OBSOLETE
	MATERIAL_VAR_SUPPRESS_DECALS		  = (1 << 26),
	MATERIAL_VAR_HALFLAMBERT			  = (1 << 27),
	MATERIAL_VAR_WIREFRAME                = (1 << 28),
	MATERIAL_VAR_ALLOWALPHATOCOVERAGE     = (1 << 29),
	MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY  = (1 << 30),
	MATERIAL_VAR_VERTEXFOG				  = (1 << 31),

	// NOTE: Only add flags here that either should be read from
	// .vmts or can be set directly from client code. Other, internal
	// flags should to into the flag enum in IMaterialInternal.h
};

enum MaterialVarType_t_csgo 
{ 
	MATERIAL_VAR_TYPE_FLOAT = 0,
	MATERIAL_VAR_TYPE_STRING,
	MATERIAL_VAR_TYPE_VECTOR,
	MATERIAL_VAR_TYPE_TEXTURE,
	MATERIAL_VAR_TYPE_INT,
	MATERIAL_VAR_TYPE_FOURCC,
	MATERIAL_VAR_TYPE_UNDEFINED,
	MATERIAL_VAR_TYPE_MATRIX,
	MATERIAL_VAR_TYPE_MATERIAL,
};

class IMaterialVar_csgo
{
protected:
	// base data and accessors
	char* m_pStringVal;
	int m_intVal;
	Vector4D_csgo m_VecVal;

	// member data. total = 4 bytes
	uint8 m_Type : 4;

	//
	// more we don't carea about atm.

public:
	//
	// Virtuals:

	// 000:
	virtual ITexture_csgo *GetTextureValue( void ) = 0;

	virtual void _UNKOWN_001(void) = 0;

	// 002:
	virtual char const *	GetName( void ) const = 0;

	// 003:
	virtual MaterialVarSym_t_csgo	GetNameAsSymbol() const = 0;

	// 004:
	virtual void			SetFloatValue( float val ) = 0;
	
	// 005:
	virtual void			SetIntValue( int val ) = 0;
	
	// 006:
	virtual void			SetStringValue( char const *val ) = 0;

	// 007:
	virtual char const *	GetStringValue( void ) const = 0;

	virtual void _UNKOWN_008(void) = 0;
	virtual void _UNKOWN_009(void) = 0;
	virtual void _UNKOWN_010(void) = 0;
	virtual void _UNKOWN_011(void) = 0;
	virtual void _UNKOWN_012(void) = 0;
	virtual void _UNKOWN_013(void) = 0;
	virtual void _UNKOWN_014(void) = 0;
	virtual void _UNKOWN_015(void) = 0;

	// 016:
	virtual IMaterial_csgo * GetMaterialValue( void ) = 0;
	
	// 017:
	virtual void SetMaterialValue( IMaterial_csgo * ) = 0;

	// 018:
	virtual bool IsDefined() const = 0;

	// 019:
	virtual void SetUndefined() = 0;

	virtual void _UNKOWN_020(void) = 0;
	virtual void _UNKOWN_021(void) = 0;
	virtual void _UNKOWN_022(void) = 0;
	virtual void _UNKOWN_023(void) = 0;
	virtual void _UNKOWN_024(void) = 0;
	
	// 025:
	virtual IMaterial_csgo * GetOwningMaterial() = 0;

	virtual void _UNKOWN_026(void) = 0; // SetVecComponentValue

protected:

	// 027:
	virtual int				GetIntValueInternal( void ) const = 0;

	// 028:
	virtual float			GetFloatValueInternal( void ) const = 0;

	// 030:
	virtual float const*	GetVecValueInternal( ) const = 0;

	// 029:
	virtual void			GetVecValueInternal( float *val, int numcomps ) const = 0;

	// 030:
	virtual int				VectorSizeInternal() const = 0;

	//
	// more we don't carea about atm.

public:
	FORCEINLINE MaterialVarType_t_csgo GetType( void ) const
	{
		return ( MaterialVarType_t_csgo )m_Type;
	}

	FORCEINLINE bool IsTexture() const
	{
		return m_Type == MATERIAL_VAR_TYPE_TEXTURE;
	}

#ifndef FAST_MATERIALVAR_ACCESS
	FORCEINLINE int GetIntValue( void ) const
	{
		return GetIntValueInternal();
	}

	FORCEINLINE float GetFloatValue( void ) const
	{
		return GetFloatValueInternal();
	}

	FORCEINLINE float const* GetVecValue( ) const
	{
		return GetVecValueInternal();
	}

	FORCEINLINE void GetVecValue( float *val, int numcomps ) const 
	{
		return GetVecValueInternal( val, numcomps );
	}

	FORCEINLINE int VectorSize() const
	{
		return VectorSizeInternal();
	}
#endif

};

// CBasePerMaterialContextData_csgo ////////////////////////////////////////////

class CBasePerMaterialContextData_csgo								
{
public:
	uint32 m_nVarChangeID;
	bool m_bMaterialVarsChanged;							// set by mat system when material vars change. shader should rehtink and then clear the var

	FORCEINLINE CBasePerMaterialContextData_csgo( void )
	{
		m_bMaterialVarsChanged = true;
		m_nVarChangeID = 0xffffffff;
	}

	// virtual destructor so that derived classes can have their own data to be cleaned up on
	// delete of material
	virtual ~CBasePerMaterialContextData_csgo( void )
	{
	}
};

// CommandBufferCommand_t_csgo /////////////////////////////////////////////////

enum CommandBufferCommand_t_csgo
{
	// flow control commands.
	CBCMD_END = 0,									// end of stream
	CBCMD_JUMP,										// int cmd, void *adr. jump to another
													// stream. Can be used to implement
													// non-sequentially allocated storage
	CBCMD_JSR,										// int cmd, void *adr. subroutine call to another stream.

	// constant setting commands
	CBCMD_SET_PIXEL_SHADER_FLOAT_CONST,				// int cmd,int first_reg, int nregs, float values[nregs*4]


	CBCMD_SET_VERTEX_SHADER_FLOAT_CONST,			// int cmd,int first_reg, int nregs, float values[nregs*4]
	CBCMD_SET_VERTEX_SHADER_FLOAT_CONST_REF,		// int cmd,int first_reg, int nregs, &float values[nregs*4]
	CBCMD_SETPIXELSHADERFOGPARAMS,					// int cmd, int regdest
	CBCMD_STORE_EYE_POS_IN_PSCONST,					// int cmd, int regdest
	CBCMD_SET_DEPTH_FEATHERING_CONST,				// int cmd, int constant register, float blend scale

	// texture binding
	CBCMD_BIND_STANDARD_TEXTURE,					// cmd, sampler, texture id
	CBCMD_BIND_SHADERAPI_TEXTURE_HANDLE,			// cmd, sampler, texture handle

	// shaders
	CBCMD_SET_PSHINDEX,								// cmd, idx
	CBCMD_SET_VSHINDEX,								// cmd, idx

	CBCMD_SET_VERTEX_SHADER_FLASHLIGHT_STATE,		// cmd, int first_reg (for worldToTexture matrix)
	CBCMD_SET_PIXEL_SHADER_FLASHLIGHT_STATE,		// cmd, int color reg, int atten reg, int origin reg, sampler (for flashlight texture)

	CBCMD_SET_PIXEL_SHADER_UBERLIGHT_STATE,			// cmd

	CBCMD_SET_VERTEX_SHADER_NEARZFARZ_STATE,		// cmd
};

// CBaseShader_csgo ////////////////////////////////////////////////////////////

struct ShaderParamInfo_t_csgo;
class IShaderInit_csgo;
class CBasePerInstanceContextData_csgo;

class IBaseShader_csgo abstract
{
	// Returns the shader name
	virtual char const* GetName( ) const = 0;

	// returns the shader fallbacks
	virtual char const* GetFallbackShader( IMaterialVar_csgo** params ) const = 0;

	// Shader parameters
	virtual int GetParamCount( ) const = 0;
	virtual const ShaderParamInfo_t_csgo & GetParamInfo( int paramIndex ) const = 0;

	virtual void InitShaderParams( IMaterialVar_csgo** ppParams, const char *pMaterialName ) = 0;

	virtual void InitShaderInstance( IMaterialVar_csgo ** ppParams, IShaderInit_csgo *pShaderInit, const char *pMaterialName, const char *pTextureGroupName ) = 0;

	virtual void DrawElements( IMaterialVar_csgo **params, int nModulationFlags, IShaderShadow_csgo* pShaderShadow, IShaderDynamicAPI_csgo* pShaderAPI,
								VertexCompressionType_t_csgo vertexCompression, CBasePerMaterialContextData_csgo **pContext, CBasePerInstanceContextData_csgo** pInstanceDataPtr ) = 0;

	virtual void _Unknown_007_SomewhatDrawElements(
		unsigned __int32 * unkDataPtr1,
		unsigned __int32 * unkDataPtr2,
		void * unkClass1, // related to VertexBuffer
		IShaderDynamicAPI_csgo* pShaderAPI,
		unsigned __int32 unkData2,
		CBasePerMaterialContextData_csgo **pContext,
		unsigned __int32 unkData3
		) = 0;

	virtual int ComputeModulationFlags( IMaterialVar_csgo** params, IShaderDynamicAPI_csgo* pShaderAPI ) = 0;

	virtual bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar_csgo **params, bool bCheckSpecificToThisFrame = true ) const;

	virtual bool NeedsFullFrameBufferTexture( IMaterialVar_csgo	 **params, bool bCheckSpecificToThisFrame = true ) const;

	virtual bool IsTranslucent( IMaterialVar_csgo **params ) const;

	virtual int GetFlags() const = 0;

	virtual void _Unknown_013_SetTexturGroupName( const char *pTextureGroupName );

	virtual void _Unknown_014_SetModulationFlags( int nModulationFlags );
	
	virtual void OnInitShaderParams( IMaterialVar_csgo** ppParams, const char *pMaterialName ) {}
	
	virtual void OnInitShaderInstance( IMaterialVar_csgo** ppParams, IShaderInit_csgo *pShaderInit, const char *pMaterialName ) = 0;
	
	virtual void OnDrawElements( IMaterialVar_csgo **params, IShaderShadow_csgo* pShaderShadow, IShaderDynamicAPI_csgo* pShaderAPI, VertexCompressionType_t_csgo vertexCompression, CBasePerMaterialContextData_csgo **pContextDataPtr ) = 0;

};


////////////////////////////////////////////////////////////////////////////////

namespace CSGO {
	class CBaseHandle;
}

class IClientEntity_csgo;
class ClientClass_csgo;
class IClientNetworkable_csgo;
class IClientUnknown_csgo;
struct EntityCacheInfo_t_csgo;

//-----------------------------------------------------------------------------
// Purpose: Exposes IClientEntity's to engine
//-----------------------------------------------------------------------------
class IClientEntityList_csgo abstract
{
public:
	// Get IClientNetworkable interface for specified entity
	virtual IClientNetworkable_csgo*	GetClientNetworkable( int entnum ) = 0;
	virtual IClientNetworkable_csgo*	GetClientNetworkableFromHandle( CSGO::CBaseHandle hEnt ) = 0;
	virtual IClientUnknown_csgo*		GetClientUnknownFromHandle( CSGO::CBaseHandle hEnt ) = 0;

	// NOTE: This function is only a convenience wrapper.
	// It returns GetClientNetworkable( entnum )->GetIClientEntity().
	virtual IClientEntity_csgo*		GetClientEntity( int entnum ) = 0;
	virtual IClientEntity_csgo*		GetClientEntityFromHandle( CSGO::CBaseHandle hEnt ) = 0;

	// Returns number of entities currently in use
	virtual int					NumberOfEntities( bool bIncludeNonNetworkable ) = 0;

	// Returns highest index actually used
	virtual int					GetHighestEntityIndex( void ) = 0;

	// Sizes entity list to specified size
	virtual void				SetMaxEntities( int maxents ) = 0;
	virtual int					GetMaxEntities( ) = 0;
	virtual EntityCacheInfo_t_csgo	*GetClientNetworkableArray() = 0;
};

extern IClientEntityList_csgo * g_Entitylist_csgo;

#define VCLIENTENTITYLIST_INTERFACE_VERSION_CSGO "VClientEntityList003"

// An IHandleEntity-derived class can go into an entity list and use ehandles.
class IHandleEntity_csgo
{
public:
	virtual ~IHandleEntity_csgo() {}
	virtual void SetRefEHandle( const CSGO::CBaseHandle &handle ) = 0;
	virtual const CSGO::CBaseHandle& GetRefEHandle() const = 0;
#ifdef _X360
	IHandleEntity() :
		m_bIsStaticProp( false )
	{
	}

	bool m_bIsStaticProp;
#endif
};

class C_BaseEntity_csgo;
class IClientRenderable_csgo;
class ICollideable_csgo;
class IClientThinkable_csgo;
class IClientModelRenderable_csgo;
class IClientAlphaProperty_csgo;

// This is the client's version of IUnknown. We may want to use a QueryInterface-like
// mechanism if this gets big.
class IClientUnknown_csgo : public IHandleEntity_csgo
{
public:
	virtual ICollideable_csgo*		GetCollideable() = 0;
	virtual IClientNetworkable_csgo*	GetClientNetworkable() = 0;
	virtual IClientRenderable_csgo*	GetClientRenderable() = 0;
	virtual IClientEntity_csgo*		GetIClientEntity() = 0;
	virtual C_BaseEntity_csgo*		GetBaseEntity() = 0;
	virtual IClientThinkable_csgo*	GetClientThinkable() = 0;
	virtual IClientModelRenderable_csgo*	GetClientModelRenderable() = 0;
	// not in csgo // virtual IClientAlphaProperty_csgo*	GetClientAlphaProperty() = 0;
};

class IClientRenderable_csgo abstract
{
	// More functions, but we don't care about this.
};

class IClientNetworkable_csgo abstract
{
	// More functions, but we don't care about this.
};

class IClientThinkable_csgo abstract
{
	// More functions, but we don't care about this.
};


class CMouthInfo_csgo;
struct SpatializationInfo_t_csgo;

//-----------------------------------------------------------------------------
// Purpose: All client entities must implement this interface.
//-----------------------------------------------------------------------------
class IClientEntity_csgo abstract : public IClientUnknown_csgo, public IClientRenderable_csgo, public IClientNetworkable_csgo, public IClientThinkable_csgo
{
public:
	// Delete yourself.
	// not in csgo // virtual void			Release( void ) = 0;
	
	// Network origin + angles
	virtual const Vector&	GetAbsOrigin( void ) const = 0;
	virtual const QAngle&	GetAbsAngles( void ) const = 0;

	virtual CMouthInfo_csgo		*GetMouth( void ) = 0;

	// Retrieve sound spatialization info for the specified sound on this entity
	// Return false to indicate sound is not audible
	virtual bool			GetSoundSpatialization( SpatializationInfo_t_csgo& info ) = 0;

	virtual bool			IsBlurred( void ) = 0;
};

class C_BaseEntity_csgo abstract : public IClientEntity_csgo
{
public:
	// (0..14 is from IClientEntity_csgo)

	virtual void _UNKNOWN_C_BaseEntity_015(void);
	virtual void _UNKNOWN_C_BaseEntity_016(void);
	virtual void _UNKNOWN_C_BaseEntity_017(void);
	virtual void _UNKNOWN_C_BaseEntity_018(void);
	virtual void _UNKNOWN_C_BaseEntity_019(void);
	virtual void _UNKNOWN_C_BaseEntity_020(void);
	virtual void _UNKNOWN_C_BaseEntity_021(void);
	virtual void _UNKNOWN_C_BaseEntity_022(void);
	virtual void _UNKNOWN_C_BaseEntity_023(void);
	virtual void _UNKNOWN_C_BaseEntity_024(void);
	virtual void _UNKNOWN_C_BaseEntity_025(void);
	virtual void _UNKNOWN_C_BaseEntity_026(void);
	virtual void _UNKNOWN_C_BaseEntity_027(void);
	virtual void _UNKNOWN_C_BaseEntity_028(void);
	virtual void _UNKNOWN_C_BaseEntity_029(void);
	virtual void _UNKNOWN_C_BaseEntity_030(void);
	virtual void _UNKNOWN_C_BaseEntity_031(void);
	virtual void _UNKNOWN_C_BaseEntity_032(void);
	virtual void _UNKNOWN_C_BaseEntity_033(void);
	virtual void _UNKNOWN_C_BaseEntity_034(void);
	virtual void _UNKNOWN_C_BaseEntity_035(void);
	virtual void _UNKNOWN_C_BaseEntity_036(void);
	virtual void _UNKNOWN_C_BaseEntity_037(void);
	virtual void _UNKNOWN_C_BaseEntity_038(void);
	virtual void _UNKNOWN_C_BaseEntity_039(void);
	virtual void _UNKNOWN_C_BaseEntity_040(void);
	virtual void _UNKNOWN_C_BaseEntity_041(void);
	virtual void _UNKNOWN_C_BaseEntity_042(void);
	virtual void _UNKNOWN_C_BaseEntity_043(void);
	virtual void _UNKNOWN_C_BaseEntity_044(void);
	virtual void _UNKNOWN_C_BaseEntity_045(void);
	virtual void _UNKNOWN_C_BaseEntity_046(void);
	virtual void _UNKNOWN_C_BaseEntity_047(void);
	virtual void _UNKNOWN_C_BaseEntity_048(void);
	virtual void _UNKNOWN_C_BaseEntity_049(void);
	virtual void _UNKNOWN_C_BaseEntity_050(void);
	virtual void _UNKNOWN_C_BaseEntity_051(void);
	virtual void _UNKNOWN_C_BaseEntity_052(void);
	virtual void _UNKNOWN_C_BaseEntity_053(void);
	virtual void _UNKNOWN_C_BaseEntity_054(void);
	virtual void _UNKNOWN_C_BaseEntity_055(void);
	virtual void _UNKNOWN_C_BaseEntity_056(void);
	virtual void _UNKNOWN_C_BaseEntity_057(void);
	virtual void _UNKNOWN_C_BaseEntity_058(void);
	virtual void _UNKNOWN_C_BaseEntity_059(void);
	virtual void _UNKNOWN_C_BaseEntity_060(void);
	virtual void _UNKNOWN_C_BaseEntity_061(void);
	virtual void _UNKNOWN_C_BaseEntity_062(void);
	virtual void _UNKNOWN_C_BaseEntity_063(void);
	virtual void _UNKNOWN_C_BaseEntity_064(void);
	virtual void _UNKNOWN_C_BaseEntity_065(void);
	virtual void _UNKNOWN_C_BaseEntity_066(void);
	virtual void _UNKNOWN_C_BaseEntity_067(void);
	virtual void _UNKNOWN_C_BaseEntity_068(void);
	virtual void _UNKNOWN_C_BaseEntity_069(void);
	virtual void _UNKNOWN_C_BaseEntity_070(void);
	virtual void _UNKNOWN_C_BaseEntity_071(void);
	virtual void _UNKNOWN_C_BaseEntity_072(void);
	virtual void _UNKNOWN_C_BaseEntity_073(void);
	virtual void _UNKNOWN_C_BaseEntity_074(void);
	virtual void _UNKNOWN_C_BaseEntity_075(void);
	virtual void _UNKNOWN_C_BaseEntity_076(void);
	virtual void _UNKNOWN_C_BaseEntity_077(void);
	virtual void _UNKNOWN_C_BaseEntity_078(void);
	virtual void _UNKNOWN_C_BaseEntity_079(void);
	virtual void _UNKNOWN_C_BaseEntity_080(void);
	virtual void _UNKNOWN_C_BaseEntity_081(void);
	virtual void _UNKNOWN_C_BaseEntity_082(void);
	virtual void _UNKNOWN_C_BaseEntity_083(void);
	virtual void _UNKNOWN_C_BaseEntity_084(void);
	virtual void _UNKNOWN_C_BaseEntity_085(void);
	virtual void _UNKNOWN_C_BaseEntity_086(void);
	virtual void _UNKNOWN_C_BaseEntity_087(void);
	virtual void _UNKNOWN_C_BaseEntity_088(void);
	virtual void _UNKNOWN_C_BaseEntity_089(void);
	virtual void _UNKNOWN_C_BaseEntity_090(void);
	virtual void _UNKNOWN_C_BaseEntity_091(void);
	virtual void _UNKNOWN_C_BaseEntity_092(void);
	virtual void _UNKNOWN_C_BaseEntity_093(void);
	virtual void _UNKNOWN_C_BaseEntity_094(void);
	virtual void _UNKNOWN_C_BaseEntity_095(void);
	virtual void _UNKNOWN_C_BaseEntity_096(void);
	virtual void _UNKNOWN_C_BaseEntity_097(void);
	virtual void _UNKNOWN_C_BaseEntity_098(void);
	virtual void _UNKNOWN_C_BaseEntity_099(void);
	virtual void _UNKNOWN_C_BaseEntity_100(void);
	virtual void _UNKNOWN_C_BaseEntity_101(void);
	virtual void _UNKNOWN_C_BaseEntity_102(void);
	virtual void _UNKNOWN_C_BaseEntity_103(void);
	virtual void _UNKNOWN_C_BaseEntity_104(void);
	virtual void _UNKNOWN_C_BaseEntity_105(void);
	virtual void _UNKNOWN_C_BaseEntity_106(void);
	virtual void _UNKNOWN_C_BaseEntity_107(void);
	virtual void _UNKNOWN_C_BaseEntity_108(void);
	virtual void _UNKNOWN_C_BaseEntity_109(void);
	virtual void _UNKNOWN_C_BaseEntity_110(void);
	
	virtual float					GetInterpolationAmount(int flags); //:111

	virtual void _UNKNOWN_C_BaseEntity_112(void);
	virtual void _UNKNOWN_C_BaseEntity_113(void);
	virtual void _UNKNOWN_C_BaseEntity_114(void);
	virtual void _UNKNOWN_C_BaseEntity_115(void);
	virtual void _UNKNOWN_C_BaseEntity_116(void);
	virtual void _UNKNOWN_C_BaseEntity_117(void);
	virtual void _UNKNOWN_C_BaseEntity_118(void);
	virtual void _UNKNOWN_C_BaseEntity_119(void);
	virtual void _UNKNOWN_C_BaseEntity_120(void);
	virtual void _UNKNOWN_C_BaseEntity_121(void);
	virtual void _UNKNOWN_C_BaseEntity_122(void);
	virtual void _UNKNOWN_C_BaseEntity_123(void);
	virtual void _UNKNOWN_C_BaseEntity_124(void);
	virtual void _UNKNOWN_C_BaseEntity_125(void);
	virtual void _UNKNOWN_C_BaseEntity_126(void);
	virtual void _UNKNOWN_C_BaseEntity_127(void);
	virtual void _UNKNOWN_C_BaseEntity_128(void);
	virtual void _UNKNOWN_C_BaseEntity_129(void);
	virtual void _UNKNOWN_C_BaseEntity_130(void);
	virtual void _UNKNOWN_C_BaseEntity_131(void);
	virtual void _UNKNOWN_C_BaseEntity_132(void);
	virtual void _UNKNOWN_C_BaseEntity_133(void);
	virtual void _UNKNOWN_C_BaseEntity_134(void);
	virtual void _UNKNOWN_C_BaseEntity_135(void);
	virtual void _UNKNOWN_C_BaseEntity_136(void);
	virtual void _UNKNOWN_C_BaseEntity_137(void);
	virtual void _UNKNOWN_C_BaseEntity_138(void);

	// 139
	virtual char const				*GetClassname( void );
	
	//char const						*GetDebugName( void );
	
	// 140
	virtual const char				*GetPlayerName() const { return 0; }

	virtual void _UNKNOWN_C_BaseEntity_141(void);
	virtual void _UNKNOWN_C_BaseEntity_142(void);
	virtual void _UNKNOWN_C_BaseEntity_143(void);
	virtual void _UNKNOWN_C_BaseEntity_144(void);
	virtual void _UNKNOWN_C_BaseEntity_145(void);
	virtual void _UNKNOWN_C_BaseEntity_146(void);
	virtual void _UNKNOWN_C_BaseEntity_147(void);
	virtual void _UNKNOWN_C_BaseEntity_148(void);
	virtual void _UNKNOWN_C_BaseEntity_149(void);
	virtual void _UNKNOWN_C_BaseEntity_150(void);
	virtual void _UNKNOWN_C_BaseEntity_151(void);
	virtual void _UNKNOWN_C_BaseEntity_152(void);
	virtual void _UNKNOWN_C_BaseEntity_153(void);
	virtual void _UNKNOWN_C_BaseEntity_154(void);
	virtual void _UNKNOWN_C_BaseEntity_155(void);
	virtual void _UNKNOWN_C_BaseEntity_156(void);
	virtual void _UNKNOWN_C_BaseEntity_157(void);
	virtual void _UNKNOWN_C_BaseEntity_158(void);
	virtual void _UNKNOWN_C_BaseEntity_159(void);
	virtual void _UNKNOWN_C_BaseEntity_160(void);
	virtual void _UNKNOWN_C_BaseEntity_161(void);
	virtual void _UNKNOWN_C_BaseEntity_162(void);

	// Returns the eye point + angles (used for viewing + shooting)
	virtual Vector			EyePosition( void ); // :163
	virtual const QAngle&	EyeAngles( void );		// Direction of eyes // :164
	virtual const QAngle&	LocalEyeAngles( void );	// Direction of eyes in local space (pl.v_angle) // :165

public:
	const char	*GetEntityName();

private:
	int _pad_000[82];
	char m_iName[260];
};

inline const char *C_BaseEntity_csgo::GetEntityName() 
{ 
	return m_iName; 
}

namespace CSGO {

#define IClientRenderable IClientRenderable_csgo
#define IBaseFileSystem IBaseFileSystem_csgo

class IHandleEntity;


// How many bits to use to encode an edict.
#define	SOURCESDK_CSGO_MAX_EDICT_BITS				11			// # of bits needed to represent max edicts
// Max # of edicts in a level
#define	SOURCESDK_CSGO_MAX_EDICTS					(1<<MAX_EDICT_BITS)

// Used for networking ehandles.
#define SOURCESDK_CSGO_NUM_ENT_ENTRY_BITS		(SOURCESDK_CSGO_MAX_EDICT_BITS + 2)
#define SOURCESDK_CSGO_NUM_ENT_ENTRIES			(1 << SOURCESDK_CSGO_NUM_ENT_ENTRY_BITS)
#define SOURCESDK_CSGO_INVALID_EHANDLE_INDEX	0xFFFFFFFF

#define SOURCESDK_CSGO_NUM_SERIAL_NUM_BITS		16 // (32 - NUM_ENT_ENTRY_BITS)
#define SOURCESDK_CSGO_NUM_SERIAL_NUM_SHIFT_BITS (32 - SOURCESDK_CSGO_NUM_SERIAL_NUM_BITS)
#define SOURCESDK_CSGO_ENT_ENTRY_MASK	(( 1 << SOURCESDK_CSGO_NUM_SERIAL_NUM_BITS) - 1)


// -------------------------------------------------------------------------------------------------- //
// CBaseHandle.
// -------------------------------------------------------------------------------------------------- //

class CBaseHandle
{
friend class CBaseEntityList;

public:

	CBaseHandle();
	CBaseHandle( const CBaseHandle &other );
	CBaseHandle( unsigned long value );
	CBaseHandle( int iEntry, int iSerialNumber );

	void Init( int iEntry, int iSerialNumber );
	void Term();

	// Even if this returns true, Get() still can return return a non-null value.
	// This just tells if the handle has been initted with any values.
	bool IsValid() const;

	int GetEntryIndex() const;
	int GetSerialNumber() const;

	int ToInt() const;
	bool operator !=( const CBaseHandle &other ) const;
	bool operator ==( const CBaseHandle &other ) const;
	bool operator ==( const IHandleEntity* pEnt ) const;
	bool operator !=( const IHandleEntity* pEnt ) const;
	bool operator <( const CBaseHandle &other ) const;
	bool operator <( const IHandleEntity* pEnt ) const;

	// Assign a value to the handle.
	const CBaseHandle& operator=( const IHandleEntity *pEntity );
	const CBaseHandle& Set( const IHandleEntity *pEntity );

	// Use this to dereference the handle.
	// Note: this is implemented in game code (ehandle.h)
	IHandleEntity* Get() const;

	void AfxAssign(const CBaseHandle &other)
	{
		m_Index = other.m_Index;
	}

protected:
	// The low NUM_SERIAL_BITS hold the index. If this value is less than MAX_EDICTS, then the entity is networkable.
	// The high NUM_SERIAL_NUM_BITS bits are the serial number.
	unsigned long	m_Index;
};

inline CBaseHandle::CBaseHandle()
{
	m_Index = SOURCESDK_CSGO_INVALID_EHANDLE_INDEX;
}

inline CBaseHandle::CBaseHandle( const CBaseHandle &other )
{
	m_Index = other.m_Index;
}

inline CBaseHandle::CBaseHandle( unsigned long value )
{
	m_Index = value;
}

inline CBaseHandle::CBaseHandle( int iEntry, int iSerialNumber )
{
	Init( iEntry, iSerialNumber );
}

inline void CBaseHandle::Init( int iEntry, int iSerialNumber )
{
	Assert( iEntry >= 0 && (iEntry & SOURCESDK_CSGO_ENT_ENTRY_MASK) == iEntry);
	Assert( iSerialNumber >= 0 && iSerialNumber < (1 << SOURCESDK_CSGO_NUM_SERIAL_NUM_BITS) );

	m_Index = iEntry | (iSerialNumber << SOURCESDK_CSGO_NUM_SERIAL_NUM_SHIFT_BITS);
}

inline void CBaseHandle::Term()
{
	m_Index = SOURCESDK_CSGO_INVALID_EHANDLE_INDEX;
}

inline bool CBaseHandle::IsValid() const
{
	return m_Index != SOURCESDK_CSGO_INVALID_EHANDLE_INDEX;
}

inline int CBaseHandle::GetEntryIndex() const
{
	// There is a hack here: due to a bug in the original implementation of the 
	// entity handle system, an attempt to look up an invalid entity index in 
	// certain cirumstances might fall through to the the mask operation below.
	// This would mask an invalid index to be in fact a lookup of entity number
	// NUM_ENT_ENTRIES, so invalid ent indexes end up actually looking up the
	// last slot in the entities array. Since this slot is always empty, the 
	// lookup returns NULL and the expected behavior occurs through this unexpected
	// route.
	// A lot of code actually depends on this behavior, and the bug was only exposed
	// after a change to NUM_SERIAL_NUM_BITS increased the number of allowable
	// static props in the world. So the if-stanza below detects this case and 
	// retains the prior (bug-submarining) behavior.
	if ( !IsValid() )
		return SOURCESDK_CSGO_NUM_ENT_ENTRIES-1;
	return m_Index & SOURCESDK_CSGO_ENT_ENTRY_MASK;
}

inline int CBaseHandle::GetSerialNumber() const
{
	return m_Index >> SOURCESDK_CSGO_NUM_SERIAL_NUM_SHIFT_BITS;
}

inline int CBaseHandle::ToInt() const
{
	return (int)m_Index;
}

inline bool CBaseHandle::operator !=( const CBaseHandle &other ) const
{
	return m_Index != other.m_Index;
}

inline bool CBaseHandle::operator ==( const CBaseHandle &other ) const
{
	return m_Index == other.m_Index;
}

class CUtlBuffer;

template< class T, class I = int >
/// <remarks>Warning, only required elements declared and defined!</remarks>
class CUtlMemory
{
public:
	//
	// We don't need this
};

template< class T, class A = CUtlMemory<T> >
/// <remarks>Warning, only required elements declared and defined!</remarks>
class CUtlVector
{
	typedef A CAllocator;
public:
	typedef T ElemType_t;

protected:
	CAllocator m_Memory;
	int m_Size;

#ifndef _X360
	// For easier access to the elements through the debugger
	// it's in release builds so this can be used in libraries correctly
	T *m_pElements;

#else
#endif
};

typedef unsigned __int16 uint16;

typedef void * FileHandle_t;

typedef unsigned char byte;

// callback to evaluate a $<symbol> during evaluation, return true or false
typedef bool(*GetSymbolProc_t)(const char *pKey);

typedef struct color32_s
{
	bool operator!=(const struct color32_s &other) const;

	byte r, g, b, a;

	// assign and copy by using the whole register rather
	// than byte-by-byte copy. (No, the compiler is not
	// smart enough to do this for you. /FAcs if you 
	// don't believe me.)
	inline unsigned *asInt(void) { return reinterpret_cast<unsigned*>(this); }
	inline const unsigned *asInt(void) const { return reinterpret_cast<const unsigned*>(this); }
	// This thing is in a union elsewhere, and union members can't have assignment
	// operators, so you have to explicitly assign using this, or be slow. SUCK.
	inline void Copy(const color32_s &rhs)
	{
		*asInt() = *rhs.asInt();
	}

} color32;

inline bool color32::operator!=(const color32 &other) const
{
	return r != other.r || g != other.g || b != other.b || a != other.a;
}

//-----------------------------------------------------------------------------
// Purpose: Basic handler for an rgb set of colors
//			This class is fully inline
//-----------------------------------------------------------------------------
class Color
{
public:
	// constructors
	Color()
	{
		*((int *)this) = 0;
	}
	Color(int _r, int _g, int _b)
	{
		SetColor(_r, _g, _b, 0);
	}
	Color(int _r, int _g, int _b, int _a)
	{
		SetColor(_r, _g, _b, _a);
	}

	// set the color
	// r - red component (0-255)
	// g - green component (0-255)
	// b - blue component (0-255)
	// a - alpha component, controls transparency (0 - transparent, 255 - opaque);
	void SetColor(int _r, int _g, int _b, int _a = 0)
	{
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}

	void GetColor(int &_r, int &_g, int &_b, int &_a) const
	{
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	void SetRawColor(int color32)
	{
		*((int *)this) = color32;
	}

	int GetRawColor() const
	{
		return *((int *)this);
	}

	inline int r() const { return _color[0]; }
	inline int g() const { return _color[1]; }
	inline int b() const { return _color[2]; }
	inline int a() const { return _color[3]; }

	unsigned char &operator[](int index)
	{
		return _color[index];
	}

	const unsigned char &operator[](int index) const
	{
		return _color[index];
	}

	bool operator == (const Color &rhs) const
	{
		return (*((int *)this) == *((int *)&rhs));
	}

	bool operator != (const Color &rhs) const
	{
		return !(operator==(rhs));
	}

	Color &operator=(const Color &rhs)
	{
		SetRawColor(rhs.GetRawColor());
		return *this;
	}

	Color &operator=(const color32 &rhs)
	{
		_color[0] = rhs.r;
		_color[1] = rhs.g;
		_color[2] = rhs.b;
		_color[3] = rhs.a;
		return *this;
	}

	color32 ToColor32() const
	{
		color32 newColor;
		newColor.r = _color[0];
		newColor.g = _color[1];
		newColor.b = _color[2];
		newColor.a = _color[3];
		return newColor;
	}

private:
	unsigned char _color[4];
};

class IKeyValuesDumpContext;

//-----------------------------------------------------------------------------
// Purpose: Simple recursive data access class
//			Used in vgui for message parameters and resource files
//			Destructor deletes all child KeyValues nodes
//			Data is stored in key (string names) - (string/int/float)value pairs called nodes.
//
//	About KeyValues Text File Format:

//	It has 3 control characters '{', '}' and '"'. Names and values may be quoted or
//	not. The quote '"' charater must not be used within name or values, only for
//	quoting whole tokens. You may use escape sequences wile parsing and add within a
//	quoted token a \" to add quotes within your name or token. When using Escape
//	Sequence the parser must now that by setting KeyValues::UsesEscapeSequences( true ),
//	which it's off by default. Non-quoted tokens ends with a whitespace, '{', '}' and '"'.
//	So you may use '{' and '}' within quoted tokens, but not for non-quoted tokens.
//  An open bracket '{' after a key name indicates a list of subkeys which is finished
//  with a closing bracket '}'. Subkeys use the same definitions recursively.
//  Whitespaces are space, return, newline and tabulator. Allowed Escape sequences
//	are \n, \t, \\, \n and \". The number character '#' is used for macro purposes 
//	(eg #include), don't use it as first charater in key names.
//-----------------------------------------------------------------------------
class KeyValues
{
public:
/*	KeyValues(const char *setName);

	//
	// AutoDelete class to automatically free the keyvalues.
	// Simply construct it with the keyvalues you allocated and it will free them when falls out of scope.
	// When you decide that keyvalues shouldn't be deleted call Assign(NULL) on it.
	// If you constructed AutoDelete(NULL) you can later assign the keyvalues to be deleted with Assign(pKeyValues).
	//
	class AutoDelete
	{
	public:
		explicit inline AutoDelete(KeyValues *pKeyValues) : m_pKeyValues(pKeyValues) {}
		explicit inline AutoDelete(const char *pchKVName) : m_pKeyValues(new KeyValues(pchKVName)) {}
		inline ~AutoDelete(void) { if (m_pKeyValues) m_pKeyValues->deleteThis(); }
		inline void Assign(KeyValues *pKeyValues) { m_pKeyValues = pKeyValues; }
		KeyValues *operator->() { return m_pKeyValues; }
		operator KeyValues *() { return m_pKeyValues; }
	private:
		AutoDelete(AutoDelete const &x); // forbid
		AutoDelete & operator= (AutoDelete const &x); // forbid
	protected:
		KeyValues *m_pKeyValues;
	};

	//
	// AutoDeleteInline is useful when you want to hold your keyvalues object inside
	// and delete it right after using.
	// You can also pass temporary KeyValues object as an argument to a function by wrapping it into KeyValues::AutoDeleteInline
	// instance:   call_my_function( KeyValues::AutoDeleteInline( new KeyValues( "test" ) ) )
	//
	class AutoDeleteInline : public AutoDelete
	{
	public:
		explicit inline AutoDeleteInline(KeyValues *pKeyValues) : AutoDelete(pKeyValues) {}
		inline operator KeyValues *() const { return m_pKeyValues; }
		inline KeyValues * Get() const { return m_pKeyValues; }
	};

	// Quick setup constructors
	KeyValues(const char *setName, const char *firstKey, const char *firstValue);
	KeyValues(const char *setName, const char *firstKey, const wchar_t *firstValue);
	KeyValues(const char *setName, const char *firstKey, int firstValue);
	KeyValues(const char *setName, const char *firstKey, const char *firstValue, const char *secondKey, const char *secondValue);
	KeyValues(const char *setName, const char *firstKey, int firstValue, const char *secondKey, int secondValue);
*/
	// Section name
	const char *GetName() const;
	void SetName(const char *setName);

	// gets the name as a unique int
	int GetNameSymbol() const;
	int GetNameSymbolCaseSensitive() const;

	// File access. Set UsesEscapeSequences true, if resource file/buffer uses Escape Sequences (eg \n, \t)
	void UsesEscapeSequences(bool state); // default false
	bool LoadFromFile(IBaseFileSystem *filesystem, const char *resourceName, const char *pathID = NULL, GetSymbolProc_t pfnEvaluateSymbolProc = NULL);
	bool SaveToFile(IBaseFileSystem *filesystem, const char *resourceName, const char *pathID = NULL);

	// Read from a buffer...  Note that the buffer must be null terminated
	bool LoadFromBuffer(char const *resourceName, const char *pBuffer, IBaseFileSystem* pFileSystem = NULL, const char *pPathID = NULL, GetSymbolProc_t pfnEvaluateSymbolProc = NULL);

	// Read from a utlbuffer...
	bool LoadFromBuffer(char const *resourceName, CUtlBuffer &buf, IBaseFileSystem* pFileSystem = NULL, const char *pPathID = NULL, GetSymbolProc_t pfnEvaluateSymbolProc = NULL);

	// Find a keyValue, create it if it is not found.
	// Set bCreate to true to create the key if it doesn't already exist (which ensures a valid pointer will be returned)
	KeyValues *FindKey(const char *keyName, bool bCreate = false);
	KeyValues *FindKey(int keySymbol) const;
	KeyValues *CreateNewKey();		// creates a new key, with an autogenerated name.  name is guaranteed to be an integer, of value 1 higher than the highest other integer key name
	void AddSubKey(KeyValues *pSubkey);	// Adds a subkey. Make sure the subkey isn't a child of some other keyvalues
	void RemoveSubKey(KeyValues *subKey);	// removes a subkey from the list, DOES NOT DELETE IT
	void InsertSubKey(int nIndex, KeyValues *pSubKey); // Inserts the given sub-key before the Nth child location
	bool ContainsSubKey(KeyValues *pSubKey); // Returns true if this key values contains the specified sub key, false otherwise.
	void SwapSubKey(KeyValues *pExistingSubKey, KeyValues *pNewSubKey);	// Swaps an existing subkey for a new one, DOES NOT DELETE THE OLD ONE but takes ownership of the new one
	void ElideSubKey(KeyValues *pSubKey);	// Removes a subkey but inserts all of its children in its place, in-order (flattens a tree, like firing a manager!)

											// Key iteration.
											//
											// NOTE: GetFirstSubKey/GetNextKey will iterate keys AND values. Use the functions 
											// below if you want to iterate over just the keys or just the values.
											//
	KeyValues *GetFirstSubKey();	// returns the first subkey in the list
	KeyValues *GetNextKey();		// returns the next subkey
	void SetNextKey(KeyValues * pDat);

	//
	// These functions can be used to treat it like a true key/values tree instead of 
	// confusing values with keys.
	//
	// So if you wanted to iterate all subkeys, then all values, it would look like this:
	//     for ( KeyValues *pKey = pRoot->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	//     {
	//		   Msg( "Key name: %s\n", pKey->GetName() );
	//     }
	//     for ( KeyValues *pValue = pRoot->GetFirstValue(); pKey; pKey = pKey->GetNextValue() )
	//     {
	//         Msg( "Int value: %d\n", pValue->GetInt() );  // Assuming pValue->GetDataType() == TYPE_INT...
	//     }
	KeyValues* GetFirstTrueSubKey();
	KeyValues* GetNextTrueSubKey();

	KeyValues* GetFirstValue();	// When you get a value back, you can use GetX and pass in NULL to get the value.
	KeyValues* GetNextValue();


	// Data access
	int   GetInt(const char *keyName = NULL, int defaultValue = 0);
	uint64 GetUint64(const char *keyName = NULL, uint64 defaultValue = 0);
	float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f);
	const char *GetString(const char *keyName = NULL, const char *defaultValue = "");
	const wchar_t *GetWString(const char *keyName = NULL, const wchar_t *defaultValue = L"");
	void *GetPtr(const char *keyName = NULL, void *defaultValue = (void*)0);
	Color GetColor(const char *keyName = NULL, const Color &defaultColor = Color(0, 0, 0, 0));
	bool GetBool(const char *keyName = NULL, bool defaultValue = false) { return GetInt(keyName, defaultValue ? 1 : 0) ? true : false; }
	bool  IsEmpty(const char *keyName = NULL);

	// Data access
	int   GetInt(int keySymbol, int defaultValue = 0);
	uint64 GetUint64(int keySymbol, uint64 defaultValue = 0);
	float GetFloat(int keySymbol, float defaultValue = 0.0f);
	const char *GetString(int keySymbol, const char *defaultValue = "");
	const wchar_t *GetWString(int keySymbol, const wchar_t *defaultValue = L"");
	void *GetPtr(int keySymbol, void *defaultValue = (void*)0);
	Color GetColor(int keySymbol /* default value is all black */);
	bool GetBool(int keySymbol, bool defaultValue = false) { return GetInt(keySymbol, defaultValue ? 1 : 0) ? true : false; }
	bool  IsEmpty(int keySymbol);

	// Key writing
	void SetWString(const char *keyName, const wchar_t *value);
	void SetString(const char *keyName, const char *value);
	void SetInt(const char *keyName, int value);
	void SetUint64(const char *keyName, uint64 value);
	void SetFloat(const char *keyName, float value);
	void SetPtr(const char *keyName, void *value);
	void SetColor(const char *keyName, Color value);
	void SetBool(const char *keyName, bool value) { SetInt(keyName, value ? 1 : 0); }

	// Memory allocation (optimized)
	void *operator new(size_t iAllocSize);
	void *operator new(size_t iAllocSize, int nBlockUse, const char *pFileName, int nLine);
	void operator delete(void *pMem);
	void operator delete(void *pMem, int nBlockUse, const char *pFileName, int nLine);

	KeyValues& operator=(KeyValues& src);

	// Adds a chain... if we don't find stuff in this keyvalue, we'll look
	// in the one we're chained to.
	void ChainKeyValue(KeyValues* pChain);

	void RecursiveSaveToFile(CUtlBuffer& buf, int indentLevel);

	bool WriteAsBinary(CUtlBuffer &buffer);
	bool ReadAsBinary(CUtlBuffer &buffer);

	// Allocate & create a new copy of the keys
	KeyValues *MakeCopy(void) const;

	// Make a new copy of all subkeys, add them all to the passed-in keyvalues
	void CopySubkeys(KeyValues *pParent) const;

	// Clear out all subkeys, and the current value
	void Clear(void);

	// Data type
	enum types_t
	{
		TYPE_NONE = 0,
		TYPE_STRING,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_PTR,
		TYPE_WSTRING,
		TYPE_COLOR,
		TYPE_UINT64,
		TYPE_COMPILED_INT_BYTE,			// hack to collapse 1 byte ints in the compiled format
		TYPE_COMPILED_INT_0,			// hack to collapse 0 in the compiled format
		TYPE_COMPILED_INT_1,			// hack to collapse 1 in the compiled format
		TYPE_NUMTYPES,
	};
	types_t GetDataType(const char *keyName = NULL);

	// Virtual deletion function - ensures that KeyValues object is deleted from correct heap
	void deleteThis();

	void SetStringValue(char const *strValue);

	// unpack a key values list into a structure
	void UnpackIntoStructure(struct KeyValuesUnpackStructure const *pUnpackTable, void *pDest);

	// Process conditional keys for widescreen support.
	bool ProcessResolutionKeys(const char *pResString);

	// Dump keyvalues recursively into a dump context
	bool Dump(IKeyValuesDumpContext *pDump, int nIndentLevel = 0);

	// Merge operations describing how two keyvalues can be combined
	enum MergeKeyValuesOp_t
	{
		MERGE_KV_ALL,
		MERGE_KV_UPDATE,	// update values are copied into storage, adding new keys to storage or updating existing ones
		MERGE_KV_DELETE,	// update values specify keys that get deleted from storage
		MERGE_KV_BORROW,	// update values only update existing keys in storage, keys in update that do not exist in storage are discarded
	};
	void MergeFrom(KeyValues *kvMerge, MergeKeyValuesOp_t eOp = MERGE_KV_ALL);

	// Assign keyvalues from a string
	static KeyValues * FromString(char const *szName, char const *szStringVal, char const **ppEndOfParse = NULL);

private:
	KeyValues(KeyValues&);	// prevent copy constructor being used

							// prevent delete being called except through deleteThis()
	~KeyValues();

	KeyValues* CreateKey(const char *keyName);

	void RecursiveCopyKeyValues(KeyValues& src);
	void RemoveEverything();
	//	void RecursiveSaveToFile( IBaseFileSystem *filesystem, CUtlBuffer &buffer, int indentLevel );
	//	void WriteConvertedString( CUtlBuffer &buffer, const char *pszString );

	// NOTE: If both filesystem and pBuf are non-null, it'll save to both of them.
	// If filesystem is null, it'll ignore f.
	void RecursiveSaveToFile(IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, int indentLevel);
	void WriteConvertedString(IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, const char *pszString);

	void RecursiveLoadFromBuffer(char const *resourceName, CUtlBuffer &buf, GetSymbolProc_t pfnEvaluateSymbolProc);

	// for handling #include "filename"
	void AppendIncludedKeys(CUtlVector< KeyValues * >& includedKeys);
	void ParseIncludedKeys(char const *resourceName, const char *filetoinclude,
		IBaseFileSystem* pFileSystem, const char *pPathID, CUtlVector< KeyValues * >& includedKeys, GetSymbolProc_t pfnEvaluateSymbolProc);

	// For handling #base "filename"
	void MergeBaseKeys(CUtlVector< KeyValues * >& baseKeys);
	void RecursiveMergeKeyValues(KeyValues *baseKV);

	// NOTE: If both filesystem and pBuf are non-null, it'll save to both of them.
	// If filesystem is null, it'll ignore f.
	void InternalWrite(IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, const void *pData, int len);

	void Init();
	const char * ReadToken(CUtlBuffer &buf, bool &wasQuoted, bool &wasConditional);
	void WriteIndents(IBaseFileSystem *filesystem, FileHandle_t f, CUtlBuffer *pBuf, int indentLevel);

	void FreeAllocatedValue();
	void AllocateValueBlock(int size);

	bool ReadAsBinaryPooledFormat(CUtlBuffer &buf, IBaseFileSystem *pFileSystem, unsigned int poolKey, GetSymbolProc_t pfnEvaluateSymbolProc);

	bool EvaluateConditional(const char *pExpressionString, GetSymbolProc_t pfnEvaluateSymbolProc);

	uint32 m_iKeyName : 24;	// keyname is a symbol defined in KeyValuesSystem
	uint32 m_iKeyNameCaseSensitive1 : 8;	// 1st part of case sensitive symbol defined in KeyValueSystem

											// These are needed out of the union because the API returns string pointers
	char *m_sValue;
	wchar_t *m_wsValue;

	// we don't delete these
	union
	{
		int m_iValue;
		float m_flValue;
		void *m_pValue;
		unsigned char m_Color[4];
	};

	char	   m_iDataType;
	char	   m_bHasEscapeSequences; // true, if while parsing this KeyValue, Escape Sequences are used (default false)
	uint16	   m_iKeyNameCaseSensitive2;	// 2nd part of case sensitive symbol defined in KeyValueSystem;

	KeyValues *m_pPeer;	// pointer to next key in list
	KeyValues *m_pSub;	// pointer to Start of a new sub key list
	KeyValues *m_pChain;// Search here if it's not in our list
};

//typedef KeyValues::AutoDelete KeyValuesAD;

enum KeyValuesUnpackDestinationTypes_t
{
	UNPACK_TYPE_FLOAT,										// dest is a float
	UNPACK_TYPE_VECTOR,										// dest is a Vector
	UNPACK_TYPE_VECTOR_COLOR,								// dest is a vector, src is a color
	UNPACK_TYPE_STRING,										// dest is a char *. unpacker will allocate.
	UNPACK_TYPE_INT,										// dest is an int
	UNPACK_TYPE_FOUR_FLOATS,	 // dest is an array of 4 floats. source is a string like "1 2 3 4"
	UNPACK_TYPE_TWO_FLOATS,		 // dest is an array of 2 floats. source is a string like "1 2"
};

#define UNPACK_FIXED( kname, kdefault, dtype, ofs ) { kname, kdefault, dtype, ofs, 0 }
#define UNPACK_VARIABLE( kname, kdefault, dtype, ofs, sz ) { kname, kdefault, dtype, ofs, sz }
#define UNPACK_END_MARKER { NULL, NULL, UNPACK_TYPE_FLOAT, 0 }

struct KeyValuesUnpackStructure
{
	char const *m_pKeyName;									// null to terminate tbl
	char const *m_pKeyDefault;								// null ok
	KeyValuesUnpackDestinationTypes_t m_eDataType;			// UNPACK_TYPE_INT, ..
	size_t m_nFieldOffset;									// use offsetof to set
	size_t m_nFieldSize;									// for strings or other variable length
};

//-----------------------------------------------------------------------------
// inline methods
//-----------------------------------------------------------------------------
inline int   KeyValues::GetInt(int keySymbol, int defaultValue)
{
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->GetInt((const char *)NULL, defaultValue) : defaultValue;
}

inline uint64 KeyValues::GetUint64(int keySymbol, uint64 defaultValue)
{
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->GetUint64((const char *)NULL, defaultValue) : defaultValue;
}

inline float KeyValues::GetFloat(int keySymbol, float defaultValue)
{
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->GetFloat((const char *)NULL, defaultValue) : defaultValue;
}

inline const char *KeyValues::GetString(int keySymbol, const char *defaultValue)
{
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->GetString((const char *)NULL, defaultValue) : defaultValue;
}

inline const wchar_t *KeyValues::GetWString(int keySymbol, const wchar_t *defaultValue)
{
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->GetWString((const char *)NULL, defaultValue) : defaultValue;
}

inline void *KeyValues::GetPtr(int keySymbol, void *defaultValue)
{
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->GetPtr((const char *)NULL, defaultValue) : defaultValue;
}

inline Color KeyValues::GetColor(int keySymbol)
{
	Color defaultValue(0, 0, 0, 0);
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->GetColor() : defaultValue;
}

inline bool  KeyValues::IsEmpty(int keySymbol)
{
	KeyValues *dat = FindKey(keySymbol);
	return dat ? dat->IsEmpty() : true;
}


//
// KeyValuesDumpContext and generic implementations
//

class IKeyValuesDumpContext
{
public:
	virtual bool KvBeginKey(KeyValues *pKey, int nIndentLevel) = 0;
	virtual bool KvWriteValue(KeyValues *pValue, int nIndentLevel) = 0;
	virtual bool KvEndKey(KeyValues *pKey, int nIndentLevel) = 0;
};

class IKeyValuesDumpContextAsText : public IKeyValuesDumpContext
{
public:
	virtual bool KvBeginKey(KeyValues *pKey, int nIndentLevel);
	virtual bool KvWriteValue(KeyValues *pValue, int nIndentLevel);
	virtual bool KvEndKey(KeyValues *pKey, int nIndentLevel);

public:
	virtual bool KvWriteIndent(int nIndentLevel);
	virtual bool KvWriteText(char const *szText) = 0;
};

class CKeyValuesDumpContextAsDevMsg : public IKeyValuesDumpContextAsText
{
public:
	// Overrides developer level to dump in DevMsg, zero to dump as Msg
	CKeyValuesDumpContextAsDevMsg(int nDeveloperLevel = 1) : m_nDeveloperLevel(nDeveloperLevel) {}

public:
	virtual bool KvBeginKey(KeyValues *pKey, int nIndentLevel);
	virtual bool KvWriteText(char const *szText);

protected:
	int m_nDeveloperLevel;
};

inline bool KeyValuesDumpAsDevMsg(KeyValues *pKeyValues, int nIndentLevel = 0, int nDeveloperLevel = 1)
{
	CKeyValuesDumpContextAsDevMsg ctx(nDeveloperLevel);
	return pKeyValues->Dump(&ctx, nIndentLevel);
}

struct FlashlightState_t
{
	FlashlightState_t()
	{
		throw "not implemented";
	}
};

//-----------------------------------------------------------------------------
// Indicates the type of translucency of an unmodulated renderable
//-----------------------------------------------------------------------------
enum RenderableTranslucencyType_t
{
	RENDERABLE_IS_OPAQUE = 0,
	RENDERABLE_IS_TRANSLUCENT,
	RENDERABLE_IS_TWO_PASS,	// has both translucent and opaque sub-partsa
};

enum RenderableModelType_t
{
	RENDERABLE_MODEL_UNKNOWN_TYPE = -1,
	RENDERABLE_MODEL_ENTITY = 0,
	RENDERABLE_MODEL_STUDIOMDL,
	RENDERABLE_MODEL_STATIC_PROP,
	RENDERABLE_MODEL_BRUSH,
};

//-----------------------------------------------------------------------------
// Handles to a client shadow
//-----------------------------------------------------------------------------
typedef unsigned short ClientShadowHandle_t;

enum
{
	CLIENTSHADOW_INVALID_HANDLE = (ClientShadowHandle_t)~0
};

//-----------------------------------------------------------------------------
// Safe accessor to an entity
//-----------------------------------------------------------------------------
typedef unsigned int HTOOLHANDLE;
enum
{
	HTOOLHANDLE_INVALID = 0
};

//-----------------------------------------------------------------------------
// Flags for the creation method
//-----------------------------------------------------------------------------
enum ShadowFlags_t
{
	SHADOW_FLAGS_FLASHLIGHT = (1 << 0),
	SHADOW_FLAGS_SHADOW = (1 << 1),
	SHADOW_FLAGS_SIMPLE_PROJECTION = (1 << 2),

	// Update this if you add flags
	SHADOW_FLAGS_LAST_FLAG = SHADOW_FLAGS_SIMPLE_PROJECTION
};

//-----------------------------------------------------------------------------
// If you change this, change the flags in IClientShadowMgr.h also
//-----------------------------------------------------------------------------
enum ClientShadowFlags_t
{
	SHADOW_FLAGS_USE_RENDER_TO_TEXTURE = (SHADOW_FLAGS_LAST_FLAG << 1),
	SHADOW_FLAGS_ANIMATING_SOURCE = (SHADOW_FLAGS_LAST_FLAG << 2),
	SHADOW_FLAGS_USE_DEPTH_TEXTURE = (SHADOW_FLAGS_LAST_FLAG << 3),
	SHADOW_FLAGS_CUSTOM_DRAW = (SHADOW_FLAGS_LAST_FLAG << 4),
	// Update this if you add flags
	CLIENT_SHADOW_FLAGS_LAST_FLAG = SHADOW_FLAGS_CUSTOM_DRAW
};


//-----------------------------------------------------------------------------
// Opaque pointer returned from Find* methods, don't store this, you need to 
// Attach it to a tool entity or discard after searching
//-----------------------------------------------------------------------------
typedef void *EntitySearchResult;
typedef void *ParticleSystemSearchResult;


//-----------------------------------------------------------------------------
// Purpose: Client side tool interace (right now just handles IClientRenderables).
//  In theory could support hooking into client side entities directly
//-----------------------------------------------------------------------------
class IClientTools : public IBaseInterface
{
public:
	// Allocates or returns the handle to an entity previously found using the Find* APIs below
	virtual HTOOLHANDLE		AttachToEntity(EntitySearchResult entityToAttach) = 0;
	virtual void			DetachFromEntity(EntitySearchResult entityToDetach) = 0;

	virtual EntitySearchResult	GetEntity(HTOOLHANDLE handle) = 0;

	// Checks whether a handle is still valid.
	virtual bool			IsValidHandle(HTOOLHANDLE handle) = 0;

	// Iterates the list of entities which have been associated with tools
	virtual int				GetNumRecordables() = 0;
	virtual HTOOLHANDLE		GetRecordable(int index) = 0;

	// Iterates through ALL entities (separate list for client vs. server)
	virtual EntitySearchResult	NextEntity(EntitySearchResult currentEnt) = 0;
	EntitySearchResult			FirstEntity() { return NextEntity(NULL); }

	// Use this to turn on/off the presence of an underlying game entity
	virtual void			SetEnabled(HTOOLHANDLE handle, bool enabled) = 0;

	// Use this to tell an entity to post "state" to all listening tools
	virtual void			SetRecording(HTOOLHANDLE handle, bool recording) = 0;

	// Some entities are marked with ShouldRecordInTools false, such as ui entities, etc.
	virtual bool			ShouldRecord(HTOOLHANDLE handle) = 0;

	virtual HTOOLHANDLE		GetToolHandleForEntityByIndex(int entindex) = 0;

	virtual int				GetModelIndex(HTOOLHANDLE handle) = 0;// sub_105C8F90?
	virtual const char*		GetModelName(HTOOLHANDLE handle) = 0;
	virtual const char*		GetClassname(HTOOLHANDLE handle) = 0;

	virtual void			AddClientRenderable(IClientRenderable *pRenderable, bool bDrawWithViewModels, RenderableTranslucencyType_t nType, RenderableModelType_t nModelType = RENDERABLE_MODEL_UNKNOWN_TYPE) = 0;
	virtual void			RemoveClientRenderable(IClientRenderable *pRenderable) = 0;
	virtual void			SetTranslucencyType(IClientRenderable *pRenderable, RenderableTranslucencyType_t nType) = 0;
	virtual void			MarkClientRenderableDirty(IClientRenderable *pRenderable) = 0;
	virtual void			UpdateProjectedTexture(ClientShadowHandle_t h, bool bForce) = 0;

	virtual bool			DrawSprite(IClientRenderable *pRenderable, float scale, float frame, int rendermode, int renderfx, const Color &color, float flProxyRadius, int *pVisHandle) = 0;
	virtual void			DrawSprite(const Vector &vecOrigin, float flWidth, float flHeight, color32 color) = 0;

	virtual EntitySearchResult	GetLocalPlayer() = 0;
	virtual bool			GetLocalPlayerEyePosition(Vector& org, QAngle& ang, float &fov) = 0;

	// See ClientShadowFlags_t above
	virtual ClientShadowHandle_t CreateShadow(CBaseHandle handle, int nFlags) = 0;
	virtual void			DestroyShadow(ClientShadowHandle_t h) = 0;

	virtual ClientShadowHandle_t CreateFlashlight(const FlashlightState_t &lightState) = 0;
	virtual void			DestroyFlashlight(ClientShadowHandle_t h) = 0;
	virtual void			UpdateFlashlightState(ClientShadowHandle_t h, const FlashlightState_t &lightState) = 0;

	virtual void			AddToDirtyShadowList(ClientShadowHandle_t h, bool force = false) = 0;
	virtual void			MarkRenderToTextureShadowDirty(ClientShadowHandle_t h) = 0;

	// Global toggle for recording
	virtual void			EnableRecordingMode(bool bEnable) = 0;
	virtual bool			IsInRecordingMode() const = 0;

	// Trigger a temp entity
	virtual void			TriggerTempEntity(KeyValues *pKeyValues) = 0;

	// get owning weapon (for viewmodels)
	virtual int				GetOwningWeaponEntIndex(int entindex) = 0;
	virtual int				GetEntIndex(EntitySearchResult entityToAttach) = 0;

	virtual int				FindGlobalFlexcontroller(char const *name) = 0;
	virtual char const		*GetGlobalFlexControllerName(int idx) = 0;

	// helper for traversing ownership hierarchy
	virtual EntitySearchResult	GetOwnerEntity(EntitySearchResult currentEnt) = 0;

	// common and useful types to query for hierarchically
	virtual bool			IsPlayer(EntitySearchResult currentEnt) = 0;
	virtual bool			IsCombatCharacter(EntitySearchResult currentEnt) = 0;
	virtual bool			IsNPC(EntitySearchResult currentEnt) = 0;
	virtual bool			IsRagdoll(EntitySearchResult currentEnt) = 0;
	virtual bool			IsViewModel(EntitySearchResult currentEnt) = 0;
	virtual bool			IsViewModelOrAttachment(EntitySearchResult currentEnt) = 0;
	virtual bool			IsWeapon(EntitySearchResult currentEnt) = 0;
	virtual bool			IsSprite(EntitySearchResult currentEnt) = 0;
	virtual bool			IsProp(EntitySearchResult currentEnt) = 0;
	virtual bool			IsBrush(EntitySearchResult currentEnt) = 0;

	virtual Vector			GetAbsOrigin(HTOOLHANDLE handle) = 0;
	virtual QAngle			GetAbsAngles(HTOOLHANDLE handle) = 0;

	// This reloads a portion or all of a particle definition file.
	// It's up to the client to decide if it cares about this file
	// Use a UtlBuffer to crack the data
	virtual void			ReloadParticleDefintions(const char *pFileName, const void *pBufData, int nLen) = 0;

	// ParticleSystem iteration, query, modification
	virtual ParticleSystemSearchResult	FirstParticleSystem() { return NextParticleSystem(NULL); }
	virtual ParticleSystemSearchResult	NextParticleSystem(ParticleSystemSearchResult sr) = 0;
	virtual void						SetRecording(ParticleSystemSearchResult sr, bool bRecord) = 0;

	// Sends a mesage from the tool to the client
	virtual void			PostToolMessage(KeyValues *pKeyValues) = 0; //55

	// Indicates whether the client should render particle systems
	virtual void			EnableParticleSystems(bool bEnable) = 0; //56

	// Is the game rendering in 3rd person mode?
	virtual bool			IsRenderingThirdPerson() const = 0; //57
};

#define SOURCESDK_CSGO_VCLIENTTOOLS_INTERFACE_VERSION "VCLIENTTOOLS001"


// handle to a KeyValues key name symbol
typedef int HKeySymbol;
#define INVALID_KEY_SYMBOL (-1)

//-----------------------------------------------------------------------------
// Purpose: Interface to shared data repository for KeyValues (included in vgui_controls.lib)
//			allows for central data storage point of KeyValues symbol table
//-----------------------------------------------------------------------------
class IKeyValuesSystem
{
public:
	// registers the size of the KeyValues in the specified instance
	// so it can build a properly sized memory pool for the KeyValues objects
	// the sizes will usually never differ but this is for versioning safety
	virtual void RegisterSizeofKeyValues(int size) = 0;

	// allocates/frees a KeyValues object from the shared mempool
	virtual void *AllocKeyValuesMemory(int size) = 0;
	virtual void FreeKeyValuesMemory(void *pMem) = 0;

	// symbol table access (used for key names)
	virtual HKeySymbol GetSymbolForString(const char *name, bool bCreate = true) = 0;
	virtual const char *GetStringForSymbol(HKeySymbol symbol) = 0;

	// for debugging, adds KeyValues record into global list so we can track memory leaks
	virtual void AddKeyValuesToMemoryLeakList(void *pMem, HKeySymbol name) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList(void *pMem) = 0;
};

IKeyValuesSystem *KeyValuesSystem();


#define SOURCESDK_CSGO_MAXSTUDIOPOSEPARAM	24

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IToolSystem;
struct SpatializationInfo_t;
class KeyValues;
class CBoneList;


//-----------------------------------------------------------------------------
// Standard messages
//-----------------------------------------------------------------------------
struct EffectRecordingState_t
{
	bool m_bVisible : 1;
	bool m_bThirdPerson : 1;
	Color m_Color;
	float m_Scale;
	const char *m_pMaterialName;
	int m_nAttachment;
	Vector m_vecAttachment; // only used if m_nAttachment is -1
};

struct BaseEntityRecordingState_t
{
	BaseEntityRecordingState_t() :
		m_flTime(0.0f),
		m_pModelName(0),
		m_nOwner(-1),
		m_fEffects(0),
		m_bVisible(false),
		m_bRecordFinalVisibleSample(false),
		m_numEffects(0),
		m_pEffects(NULL),
		m_nFollowEntity(-1)
	{
		m_vecRenderOrigin.Init();
		m_vecRenderAngles.Init();
	}

	float m_flTime;
	const char *m_pModelName;
	int m_nOwner;
	int m_fEffects;
	bool m_bVisible : 1;
	bool m_bRecordFinalVisibleSample : 1;
	Vector m_vecRenderOrigin;
	QAngle m_vecRenderAngles;
	int m_nFollowEntity;

	int m_numEffects;
	EffectRecordingState_t *m_pEffects;
};

struct SpriteRecordingState_t
{
	float m_flRenderScale;
	float m_flFrame;
	int m_nRenderMode;
	bool m_nRenderFX;
	Color m_Color;
	float m_flProxyRadius;
};

struct BaseAnimatingHighLevelRecordingState_t
{
	BaseAnimatingHighLevelRecordingState_t()
		: m_bClearIkTargets(false),
		m_bIsRagdoll(false),
		m_bShouldCreateIkContext(false),
		m_nNumPoseParams(0),
		m_flCycle(0.0f),
		m_flPlaybackRate(1.0f),
		m_flCycleRate(0.0f),
		m_nFrameCount(0),
		m_bInterpEffectActive(false)
	{
	}

	bool m_bClearIkTargets;
	bool m_bIsRagdoll;
	bool m_bShouldCreateIkContext;
	int m_nNumPoseParams;

	float m_flCycle;
	float m_flPlaybackRate;
	float m_flCycleRate;
	int m_nFrameCount;

	float m_flPoseParameter[SOURCESDK_CSGO_MAXSTUDIOPOSEPARAM];

	bool m_bInterpEffectActive;
};

struct BaseAnimatingRecordingState_t
{
	BaseAnimatingHighLevelRecordingState_t m_highLevelState;

	int m_nSkin;
	int m_nBody;
	int m_nSequence;
	CBoneList *m_pBoneList;
};

struct BaseFlexRecordingState_t
{
	int m_nFlexCount;
	float *m_pDestWeight;
	Vector m_vecViewTarget;
};

struct CameraRecordingState_t
{
	bool m_bThirdPerson;
	float m_flFOV;
	Vector m_vecEyePosition;
	QAngle m_vecEyeAngles;
};

struct MonitorRecordingState_t
{
	bool	m_bActive;
	float	m_flFOV;
	bool	m_bFogEnabled;
	float	m_flFogStart;
	float	m_flFogEnd;
	Color	m_FogColor;
};

struct EntityTeleportedRecordingState_t
{
	Vector m_vecTo;
	QAngle m_qaTo;
	bool m_bTeleported;
	bool m_bViewOverride;
	matrix3x4_t m_teleportMatrix;
};

struct PortalRecordingState_t
{
	int				m_nPortalId;
	int				m_nLinkedPortalId;
	float			m_fStaticAmount;
	float			m_fSecondaryStaticAmount;
	float			m_fOpenAmount;
	bool			m_bIsPortal2; //for any set of portals, one must be portal 1, and the other portal 2. Uses different render targets
};

struct ParticleSystemCreatedState_t
{
	int				m_nParticleSystemId;
	const char *	m_pName;
	float			m_flTime;
	int				m_nOwner;
};

struct ParticleSystemDestroyedState_t
{
	int				m_nParticleSystemId;
	float			m_flTime;
};

struct ParticleSystemStopEmissionState_t
{
	int				m_nParticleSystemId;
	float			m_flTime;
	bool			m_bInfiniteOnly;
};

struct ParticleSystemSetControlPointObjectState_t
{
	int				m_nParticleSystemId;
	float			m_flTime;
	int				m_nControlPoint;
	int				m_nObject;
};

struct ParticleSystemSetControlPointPositionState_t
{
	int				m_nParticleSystemId;
	float			m_flTime;
	int				m_nControlPoint;
	Vector			m_vecPosition;
};

struct ParticleSystemSetControlPointOrientationState_t
{
	int				m_nParticleSystemId;
	float			m_flTime;
	int				m_nControlPoint;
	Quaternion		m_qOrientation;
};

#define SOURCESDK_CSGO_MAXSTUDIOBONES		256		// total bones actually used

class CBoneList
{
public:

	CBoneList();

	void Release();

	static CBoneList *Alloc();

	unsigned int GetWriteSize() const
	{
		return 2 + m_nBones * (sizeof(Vector) + sizeof(Quaternion));
	}

	// The order of these data members must be maintained in order for the server
	// demo system.  ServerDemoPacket_BaseAnimating::GetSize() depends on this.

private:
	bool		m_bShouldDelete : 1;

public:
	uint16		m_nBones : 15;
	Vector		m_vecPos[SOURCESDK_CSGO_MAXSTUDIOBONES];
	Quaternion	m_quatRot[SOURCESDK_CSGO_MAXSTUDIOBONES];
};


#undef IBaseFileSystem
#undef CBaseHandle
#undef IClientRenderable

//-----------------------------------------------------------------------------
// Purpose:	Do a an inline AddRef then return the pointer, useful when
//			returning an object from a function
//-----------------------------------------------------------------------------

#define SOURCESDK_CSGO_RetAddRef( p ) ( (p)->AddRef(), (p) )
#define SOURCESDK_CSGO_InlineAddRef( p ) ( (p)->AddRef(), (p) )

class CFunctor abstract : public IRefCounted_csgo
{
public:
	CFunctor()
	{
//#ifdef DEBUG
		m_nUserID = 0;
//#endif
	}

	virtual ~CFunctor() // new in CS:GO // :002
	{
		
	}

	virtual void operator()() = 0; // :003

	unsigned m_nUserID; // For debugging
};

class ICallQueue
{
public:
	void QueueFunctor(CFunctor *pFunctor)
	{
		QueueFunctorInternal(SOURCESDK_CSGO_RetAddRef(pFunctor));
	}

	// FUCK THESE: //FUNC_GENERATE_QUEUE_METHODS();

// private: // this won't do
public:
	virtual void QueueFunctorInternal(CFunctor *pFunctor) = 0;
};

class IGameEvent abstract
{
public:
	virtual ~IGameEvent() {};
	virtual const char *GetName() const = 0;	// get event name

	virtual bool IsReliable() const = 0; // if event handled reliable
	virtual bool IsLocal() const = 0; // if event is never networked
	virtual bool IsEmpty(const char *keyName = NULL) = 0; // check if data field exists

														  // Data access
	virtual bool GetBool(const char *keyName = NULL, bool defaultValue = false) = 0;
	virtual int GetInt(const char *keyName = NULL, int defaultValue = 0) = 0;
	virtual float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f) = 0;
	virtual void __unkown_008(void);
	virtual const char *GetString(const char *keyName = NULL, const char *defaultValue = "") = 0;

	virtual void SetBool(const char *keyName, bool value) = 0;
	virtual void SetInt(const char *keyName, int value) = 0;
	virtual void SetFloat(const char *keyName, float value) = 0;
	virtual void __unkown_013(void);
	virtual void SetString(const char *keyName, const char *value) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: Engine interface into client side prediction system
//-----------------------------------------------------------------------------
class IPrediction abstract
{
public:
	virtual			~IPrediction(void) {};

	virtual void	Init(void) = 0;
	virtual void	Shutdown(void) = 0;

	// Run prediction
	virtual void	Update
	(
		int startframe,				// World update ( un-modded ) most recently received
		bool validframe,			// Is frame data valid
		int incoming_acknowledged,	// Last command acknowledged to have been run by server (un-modded)
		int outgoing_command		// Last command (most recent) sent to server (un-modded)
	) = 0;

	// We are about to get a network update from the server.  We know the update #, so we can pull any
	//  data purely predicted on the client side and transfer it to the new from data state.
	virtual void	PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet) = 0;
	virtual void	PostEntityPacketReceived(void) = 0;
	virtual void	PostNetworkDataReceived(int commands_acknowledged) = 0;

	virtual void	OnReceivedUncompressedPacket(void) = 0;

	// The engine needs to be able to access a few predicted values
	virtual void	GetViewOrigin(Vector& org) = 0;
	virtual void	SetViewOrigin(Vector& org) = 0;
	virtual void	GetViewAngles(QAngle& ang) = 0;
	virtual void	SetViewAngles(QAngle& ang) = 0;
	virtual void	GetLocalViewAngles(QAngle& ang) = 0;
	virtual void	SetLocalViewAngles(QAngle& ang) = 0;
};

#define SOURCESDK_CSGO_VCLIENT_PREDICTION_INTERFACE_VERSION	"VClientPrediction001"


namespace vgui {

typedef unsigned int VPANEL;
typedef unsigned long HScheme;

class KeyValues;
struct DmxElementUnpackStructure_t;
class CDmxElement;

class SurfacePlat;
class IClientPanel;

//!! must be removed
class Panel;

//-----------------------------------------------------------------------------
// Purpose: interface from Client panels -> vgui panels
//-----------------------------------------------------------------------------
class IPanel : public IBaseInterface
{
public:
	virtual void Init(VPANEL vguiPanel, IClientPanel *panel) = 0;

	// methods
	virtual void SetPos(VPANEL vguiPanel, int x, int y) = 0;
	virtual void GetPos(VPANEL vguiPanel, int &x, int &y) = 0;
	virtual void SetSize(VPANEL vguiPanel, int wide, int tall) = 0;
	virtual void GetSize(VPANEL vguiPanel, int &wide, int &tall) = 0;
	virtual void SetMinimumSize(VPANEL vguiPanel, int wide, int tall) = 0;
	virtual void GetMinimumSize(VPANEL vguiPanel, int &wide, int &tall) = 0;
	virtual void SetZPos(VPANEL vguiPanel, int z) = 0;
	virtual int  GetZPos(VPANEL vguiPanel) = 0;

	virtual void GetAbsPos(VPANEL vguiPanel, int &x, int &y) = 0;
	virtual void GetClipRect(VPANEL vguiPanel, int &x0, int &y0, int &x1, int &y1) = 0;
	virtual void SetInset(VPANEL vguiPanel, int left, int top, int right, int bottom) = 0;
	virtual void GetInset(VPANEL vguiPanel, int &left, int &top, int &right, int &bottom) = 0;

	virtual void SetVisible(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsVisible(VPANEL vguiPanel) = 0;
	virtual void SetParent(VPANEL vguiPanel, VPANEL newParent) = 0;
	virtual int GetChildCount(VPANEL vguiPanel) = 0;
	virtual VPANEL GetChild(VPANEL vguiPanel, int index) = 0;
	virtual CUtlVector< VPANEL > &GetChildren(VPANEL vguiPanel) = 0;
	virtual VPANEL GetParent(VPANEL vguiPanel) = 0;
	virtual void MoveToFront(VPANEL vguiPanel) = 0;
	virtual void MoveToBack(VPANEL vguiPanel) = 0;
	virtual bool HasParent(VPANEL vguiPanel, VPANEL potentialParent) = 0;
	virtual bool IsPopup(VPANEL vguiPanel) = 0;
	virtual void SetPopup(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsFullyVisible(VPANEL vguiPanel) = 0;

	// gets the scheme this panel uses
	virtual HScheme GetScheme(VPANEL vguiPanel) = 0;
	// gets whether or not this panel should scale with screen resolution
	virtual bool IsProportional(VPANEL vguiPanel) = 0;
	// returns true if auto-deletion flag is set
	virtual bool IsAutoDeleteSet(VPANEL vguiPanel) = 0;
	// deletes the Panel * associated with the vpanel
	virtual void DeletePanel(VPANEL vguiPanel) = 0;

	// input interest
	virtual void SetKeyBoardInputEnabled(VPANEL vguiPanel, bool state) = 0;
	virtual void SetMouseInputEnabled(VPANEL vguiPanel, bool state) = 0;
	virtual bool IsKeyBoardInputEnabled(VPANEL vguiPanel) = 0;
	virtual bool IsMouseInputEnabled(VPANEL vguiPanel) = 0;

	// calculates the panels current position within the hierarchy
	virtual void Solve(VPANEL vguiPanel) = 0;

	// gets names of the object (for debugging purposes)
	virtual const char *GetName(VPANEL vguiPanel) = 0;
	virtual const char *GetClassName(VPANEL vguiPanel) = 0;

	// delivers a message to the panel
	virtual void SendMessage(VPANEL vguiPanel, KeyValues *params, VPANEL ifromPanel) = 0;

	// these pass through to the IClientPanel
	virtual void Think(VPANEL vguiPanel) = 0;
	virtual void PerformApplySchemeSettings(VPANEL vguiPanel) = 0;
	virtual void PaintTraverse(VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) = 0;
	virtual void Repaint(VPANEL vguiPanel) = 0;
	virtual VPANEL IsWithinTraverse(VPANEL vguiPanel, int x, int y, bool traversePopups) = 0;
	virtual void OnChildAdded(VPANEL vguiPanel, VPANEL child) = 0;
	virtual void OnSizeChanged(VPANEL vguiPanel, int newWide, int newTall) = 0;

	virtual void InternalFocusChanged(VPANEL vguiPanel, bool lost) = 0;
	virtual bool RequestInfo(VPANEL vguiPanel, KeyValues *outputData) = 0;
	virtual void RequestFocus(VPANEL vguiPanel, int direction = 0) = 0;
	virtual bool RequestFocusPrev(VPANEL vguiPanel, VPANEL existingPanel) = 0;
	virtual bool RequestFocusNext(VPANEL vguiPanel, VPANEL existingPanel) = 0;
	virtual VPANEL GetCurrentKeyFocus(VPANEL vguiPanel) = 0;
	virtual int GetTabPosition(VPANEL vguiPanel) = 0;

	// used by ISurface to store platform-specific data
	virtual SurfacePlat *Plat(VPANEL vguiPanel) = 0;
	virtual void SetPlat(VPANEL vguiPanel, SurfacePlat *Plat) = 0;

	// returns a pointer to the vgui controls baseclass Panel *
	// destinationModule needs to be passed in to verify that the returned Panel * is from the same module
	// it must be from the same module since Panel * vtbl may be different in each module
	virtual Panel *GetPanel(VPANEL vguiPanel, const char *destinationModule) = 0;

	virtual bool IsEnabled(VPANEL vguiPanel) = 0;
	virtual void SetEnabled(VPANEL vguiPanel, bool state) = 0;

	// Used by the drag/drop manager to always draw on top
	virtual bool IsTopmostPopup(VPANEL vguiPanel) = 0;
	virtual void SetTopmostPopup(VPANEL vguiPanel, bool state) = 0;

	virtual void SetMessageContextId(VPANEL vguiPanel, int nContextId) = 0;
	virtual int GetMessageContextId(VPANEL vguiPanel) = 0;

	virtual const DmxElementUnpackStructure_t *GetUnpackStructure(VPANEL vguiPanel) const = 0;
	virtual void OnUnserialized(VPANEL vguiPanel, CDmxElement *pElement) = 0;

	// sibling pins
	virtual void SetSiblingPin(VPANEL vguiPanel, VPANEL newSibling, byte iMyCornerToPin = 0, byte iSiblingCornerToPinTo = 0) = 0;
};

#define SOURCESDK_CSGO_VGUI_PANEL_INTERFACE_VERSION "VGUI_Panel009"

//-----------------------------------------------------------------------------
// Purpose: Wraps contextless windows system functions
//-----------------------------------------------------------------------------
class ISurface abstract : public IAppSystem_csgo
{
public:
	// call to Shutdown surface; surface can no longer be used after this is called
	virtual void Shutdown() = 0;

	// frame
	virtual void RunFrame() = 0;

	// hierarchy root
	virtual VPANEL GetEmbeddedPanel() = 0;

	// ... more we don't care about.
};

#define SOURCESDK_CSGO_VGUI_VGUI_SURFACE_INTERFACE_VERSION "VGUI_Surface031"

} // namespace vgui

} // namespace CSGO {
} // namespace SOURCESDK {
