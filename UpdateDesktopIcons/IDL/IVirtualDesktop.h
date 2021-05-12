

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Mon Jan 18 21:14:07 2038
 */
/* Compiler settings for IVirtualDesktop.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0622 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __IVirtualDesktop_h__
#define __IVirtualDesktop_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IVirtualDesktop_FWD_DEFINED__
#define __IVirtualDesktop_FWD_DEFINED__
typedef interface IVirtualDesktop IVirtualDesktop;

#endif 	/* __IVirtualDesktop_FWD_DEFINED__ */


#ifndef __IVirtualDesktopManagerInternal_FWD_DEFINED__
#define __IVirtualDesktopManagerInternal_FWD_DEFINED__
typedef interface IVirtualDesktopManagerInternal IVirtualDesktopManagerInternal;

#endif 	/* __IVirtualDesktopManagerInternal_FWD_DEFINED__ */


#ifndef __IVirtualDesktopNotification_FWD_DEFINED__
#define __IVirtualDesktopNotification_FWD_DEFINED__
typedef interface IVirtualDesktopNotification IVirtualDesktopNotification;

#endif 	/* __IVirtualDesktopNotification_FWD_DEFINED__ */


#ifndef __IVirtualDesktopNotificationService_FWD_DEFINED__
#define __IVirtualDesktopNotificationService_FWD_DEFINED__
typedef interface IVirtualDesktopNotificationService IVirtualDesktopNotificationService;

#endif 	/* __IVirtualDesktopNotificationService_FWD_DEFINED__ */


#ifndef __CVirtualDesktopManagerInternal_FWD_DEFINED__
#define __CVirtualDesktopManagerInternal_FWD_DEFINED__

#ifdef __cplusplus
typedef class CVirtualDesktopManagerInternal CVirtualDesktopManagerInternal;
#else
typedef struct CVirtualDesktopManagerInternal CVirtualDesktopManagerInternal;
#endif /* __cplusplus */

#endif 	/* __CVirtualDesktopManagerInternal_FWD_DEFINED__ */


#ifndef __CVirtualDesktopNotificationService_FWD_DEFINED__
#define __CVirtualDesktopNotificationService_FWD_DEFINED__

#ifdef __cplusplus
typedef class CVirtualDesktopNotificationService CVirtualDesktopNotificationService;
#else
typedef struct CVirtualDesktopNotificationService CVirtualDesktopNotificationService;
#endif /* __cplusplus */

#endif 	/* __CVirtualDesktopNotificationService_FWD_DEFINED__ */


#ifndef __CImmersiveShell_FWD_DEFINED__
#define __CImmersiveShell_FWD_DEFINED__

#ifdef __cplusplus
typedef class CImmersiveShell CImmersiveShell;
#else
typedef struct CImmersiveShell CImmersiveShell;
#endif /* __cplusplus */

#endif 	/* __CImmersiveShell_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "ObjectArray.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IVirtualDesktop_INTERFACE_DEFINED__
#define __IVirtualDesktop_INTERFACE_DEFINED__

/* interface IVirtualDesktop */
/* [object][uuid] */ 


EXTERN_C const IID IID_IVirtualDesktop;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FF72FFDD-BE7E-43FC-9C03-AD81681E88E4")
    IVirtualDesktop : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsViewVisible( 
            /* [in] */ IUnknown *view) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetID( 
            /* [out] */ GUID *result) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IVirtualDesktopVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVirtualDesktop * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVirtualDesktop * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVirtualDesktop * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsViewVisible )( 
            IVirtualDesktop * This,
            /* [in] */ IUnknown *view);
        
        HRESULT ( STDMETHODCALLTYPE *GetID )( 
            IVirtualDesktop * This,
            /* [out] */ GUID *result);
        
        END_INTERFACE
    } IVirtualDesktopVtbl;

    interface IVirtualDesktop
    {
        CONST_VTBL struct IVirtualDesktopVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVirtualDesktop_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVirtualDesktop_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVirtualDesktop_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVirtualDesktop_IsViewVisible(This,view)	\
    ( (This)->lpVtbl -> IsViewVisible(This,view) ) 

#define IVirtualDesktop_GetID(This,result)	\
    ( (This)->lpVtbl -> GetID(This,result) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVirtualDesktop_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_IVirtualDesktop_0000_0001 */
/* [local] */ 

typedef /* [v1_enum] */ 
enum _ADJACENT_DESKTOP
    {
        ADJ_DESK_LEFT	= 3,
        ADJ_DESK_RIGHT	= 4
    } 	ADJACENT_DESKTOP;



extern RPC_IF_HANDLE __MIDL_itf_IVirtualDesktop_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_IVirtualDesktop_0000_0001_v0_0_s_ifspec;

#ifndef __IVirtualDesktopManagerInternal_INTERFACE_DEFINED__
#define __IVirtualDesktopManagerInternal_INTERFACE_DEFINED__

/* interface IVirtualDesktopManagerInternal */
/* [object][uuid] */ 


EXTERN_C const IID IID_IVirtualDesktopManagerInternal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F31574D6-B682-4CDC-BD56-1827860ABEC6")
    IVirtualDesktopManagerInternal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
            /* [out] */ DWORD *result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop( 
            /* [in] */ IUnknown *pView,
            /* [in] */ IVirtualDesktop *desktop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops( 
            /* [in] */ IUnknown *pView) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop( 
            /* [out] */ IVirtualDesktop **desktop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDesktops( 
            /* [out] */ IObjectArray **result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop( 
            /* [in] */ IVirtualDesktop *desktop,
            /* [in] */ ADJACENT_DESKTOP direction,
            /* [out] */ IVirtualDesktop **result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SwitchDesktop( 
            /* [in] */ IVirtualDesktop *desktop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateDesktopW( 
            /* [out] */ IVirtualDesktop **result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveDesktop( 
            /* [in] */ IVirtualDesktop *pRemove,
            /* [in] */ IVirtualDesktop *fallback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindDesktop( 
            /* [in] */ GUID *desktopId,
            /* [out] */ IVirtualDesktop **result) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IVirtualDesktopManagerInternalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVirtualDesktopManagerInternal * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVirtualDesktopManagerInternal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVirtualDesktopManagerInternal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IVirtualDesktopManagerInternal * This,
            /* [out] */ DWORD *result);
        
        HRESULT ( STDMETHODCALLTYPE *MoveViewToDesktop )( 
            IVirtualDesktopManagerInternal * This,
            /* [in] */ IUnknown *pView,
            /* [in] */ IVirtualDesktop *desktop);
        
        HRESULT ( STDMETHODCALLTYPE *CanViewMoveDesktops )( 
            IVirtualDesktopManagerInternal * This,
            /* [in] */ IUnknown *pView);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentDesktop )( 
            IVirtualDesktopManagerInternal * This,
            /* [out] */ IVirtualDesktop **desktop);
        
        HRESULT ( STDMETHODCALLTYPE *GetDesktops )( 
            IVirtualDesktopManagerInternal * This,
            /* [out] */ IObjectArray **result);
        
        HRESULT ( STDMETHODCALLTYPE *GetAdjacentDesktop )( 
            IVirtualDesktopManagerInternal * This,
            /* [in] */ IVirtualDesktop *desktop,
            /* [in] */ ADJACENT_DESKTOP direction,
            /* [out] */ IVirtualDesktop **result);
        
        HRESULT ( STDMETHODCALLTYPE *SwitchDesktop )( 
            IVirtualDesktopManagerInternal * This,
            /* [in] */ IVirtualDesktop *desktop);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDesktopW )( 
            IVirtualDesktopManagerInternal * This,
            /* [out] */ IVirtualDesktop **result);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveDesktop )( 
            IVirtualDesktopManagerInternal * This,
            /* [in] */ IVirtualDesktop *pRemove,
            /* [in] */ IVirtualDesktop *fallback);
        
        HRESULT ( STDMETHODCALLTYPE *FindDesktop )( 
            IVirtualDesktopManagerInternal * This,
            /* [in] */ GUID *desktopId,
            /* [out] */ IVirtualDesktop **result);
        
        END_INTERFACE
    } IVirtualDesktopManagerInternalVtbl;

    interface IVirtualDesktopManagerInternal
    {
        CONST_VTBL struct IVirtualDesktopManagerInternalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVirtualDesktopManagerInternal_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVirtualDesktopManagerInternal_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVirtualDesktopManagerInternal_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVirtualDesktopManagerInternal_GetCount(This,result)	\
    ( (This)->lpVtbl -> GetCount(This,result) ) 

#define IVirtualDesktopManagerInternal_MoveViewToDesktop(This,pView,desktop)	\
    ( (This)->lpVtbl -> MoveViewToDesktop(This,pView,desktop) ) 

#define IVirtualDesktopManagerInternal_CanViewMoveDesktops(This,pView)	\
    ( (This)->lpVtbl -> CanViewMoveDesktops(This,pView) ) 

#define IVirtualDesktopManagerInternal_GetCurrentDesktop(This,desktop)	\
    ( (This)->lpVtbl -> GetCurrentDesktop(This,desktop) ) 

#define IVirtualDesktopManagerInternal_GetDesktops(This,result)	\
    ( (This)->lpVtbl -> GetDesktops(This,result) ) 

#define IVirtualDesktopManagerInternal_GetAdjacentDesktop(This,desktop,direction,result)	\
    ( (This)->lpVtbl -> GetAdjacentDesktop(This,desktop,direction,result) ) 

#define IVirtualDesktopManagerInternal_SwitchDesktop(This,desktop)	\
    ( (This)->lpVtbl -> SwitchDesktop(This,desktop) ) 

#define IVirtualDesktopManagerInternal_CreateDesktopW(This,result)	\
    ( (This)->lpVtbl -> CreateDesktopW(This,result) ) 

#define IVirtualDesktopManagerInternal_RemoveDesktop(This,pRemove,fallback)	\
    ( (This)->lpVtbl -> RemoveDesktop(This,pRemove,fallback) ) 

#define IVirtualDesktopManagerInternal_FindDesktop(This,desktopId,result)	\
    ( (This)->lpVtbl -> FindDesktop(This,desktopId,result) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVirtualDesktopManagerInternal_INTERFACE_DEFINED__ */


#ifndef __IVirtualDesktopNotification_INTERFACE_DEFINED__
#define __IVirtualDesktopNotification_INTERFACE_DEFINED__

/* interface IVirtualDesktopNotification */
/* [object][uuid] */ 


EXTERN_C const IID IID_IVirtualDesktopNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C179334C-4295-40D3-BEA1-C654D965605A")
    IVirtualDesktopNotification : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated( 
            /* [in] */ IVirtualDesktop *pDesktop) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin( 
            /* [in] */ IVirtualDesktop *pDesktopDestroyed,
            /* [in] */ IVirtualDesktop *pDesktopFallback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed( 
            /* [in] */ IVirtualDesktop *pDesktopDestroyed,
            /* [in] */ IVirtualDesktop *pDesktopFallback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed( 
            /* [in] */ IVirtualDesktop *pDesktopDestroyed,
            /* [in] */ IVirtualDesktop *pDesktopFallback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged( 
            /* [in] */ IUnknown *pView) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged( 
            /* [in] */ IVirtualDesktop *pDesktopOld,
            /* [in] */ IVirtualDesktop *pDesktopNew) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IVirtualDesktopNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVirtualDesktopNotification * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVirtualDesktopNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVirtualDesktopNotification * This);
        
        HRESULT ( STDMETHODCALLTYPE *VirtualDesktopCreated )( 
            IVirtualDesktopNotification * This,
            /* [in] */ IVirtualDesktop *pDesktop);
        
        HRESULT ( STDMETHODCALLTYPE *VirtualDesktopDestroyBegin )( 
            IVirtualDesktopNotification * This,
            /* [in] */ IVirtualDesktop *pDesktopDestroyed,
            /* [in] */ IVirtualDesktop *pDesktopFallback);
        
        HRESULT ( STDMETHODCALLTYPE *VirtualDesktopDestroyFailed )( 
            IVirtualDesktopNotification * This,
            /* [in] */ IVirtualDesktop *pDesktopDestroyed,
            /* [in] */ IVirtualDesktop *pDesktopFallback);
        
        HRESULT ( STDMETHODCALLTYPE *VirtualDesktopDestroyed )( 
            IVirtualDesktopNotification * This,
            /* [in] */ IVirtualDesktop *pDesktopDestroyed,
            /* [in] */ IVirtualDesktop *pDesktopFallback);
        
        HRESULT ( STDMETHODCALLTYPE *ViewVirtualDesktopChanged )( 
            IVirtualDesktopNotification * This,
            /* [in] */ IUnknown *pView);
        
        HRESULT ( STDMETHODCALLTYPE *CurrentVirtualDesktopChanged )( 
            IVirtualDesktopNotification * This,
            /* [in] */ IVirtualDesktop *pDesktopOld,
            /* [in] */ IVirtualDesktop *pDesktopNew);
        
        END_INTERFACE
    } IVirtualDesktopNotificationVtbl;

    interface IVirtualDesktopNotification
    {
        CONST_VTBL struct IVirtualDesktopNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVirtualDesktopNotification_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVirtualDesktopNotification_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVirtualDesktopNotification_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVirtualDesktopNotification_VirtualDesktopCreated(This,pDesktop)	\
    ( (This)->lpVtbl -> VirtualDesktopCreated(This,pDesktop) ) 

#define IVirtualDesktopNotification_VirtualDesktopDestroyBegin(This,pDesktopDestroyed,pDesktopFallback)	\
    ( (This)->lpVtbl -> VirtualDesktopDestroyBegin(This,pDesktopDestroyed,pDesktopFallback) ) 

#define IVirtualDesktopNotification_VirtualDesktopDestroyFailed(This,pDesktopDestroyed,pDesktopFallback)	\
    ( (This)->lpVtbl -> VirtualDesktopDestroyFailed(This,pDesktopDestroyed,pDesktopFallback) ) 

#define IVirtualDesktopNotification_VirtualDesktopDestroyed(This,pDesktopDestroyed,pDesktopFallback)	\
    ( (This)->lpVtbl -> VirtualDesktopDestroyed(This,pDesktopDestroyed,pDesktopFallback) ) 

#define IVirtualDesktopNotification_ViewVirtualDesktopChanged(This,pView)	\
    ( (This)->lpVtbl -> ViewVirtualDesktopChanged(This,pView) ) 

#define IVirtualDesktopNotification_CurrentVirtualDesktopChanged(This,pDesktopOld,pDesktopNew)	\
    ( (This)->lpVtbl -> CurrentVirtualDesktopChanged(This,pDesktopOld,pDesktopNew) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVirtualDesktopNotification_INTERFACE_DEFINED__ */


#ifndef __IVirtualDesktopNotificationService_INTERFACE_DEFINED__
#define __IVirtualDesktopNotificationService_INTERFACE_DEFINED__

/* interface IVirtualDesktopNotificationService */
/* [object][uuid] */ 


EXTERN_C const IID IID_IVirtualDesktopNotificationService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
    IVirtualDesktopNotificationService : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Register( 
            /* [in] */ IVirtualDesktopNotification *pNotification,
            /* [out] */ DWORD *cookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unregister( 
            DWORD cookie) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IVirtualDesktopNotificationServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVirtualDesktopNotificationService * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVirtualDesktopNotificationService * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVirtualDesktopNotificationService * This);
        
        HRESULT ( STDMETHODCALLTYPE *Register )( 
            IVirtualDesktopNotificationService * This,
            /* [in] */ IVirtualDesktopNotification *pNotification,
            /* [out] */ DWORD *cookie);
        
        HRESULT ( STDMETHODCALLTYPE *Unregister )( 
            IVirtualDesktopNotificationService * This,
            DWORD cookie);
        
        END_INTERFACE
    } IVirtualDesktopNotificationServiceVtbl;

    interface IVirtualDesktopNotificationService
    {
        CONST_VTBL struct IVirtualDesktopNotificationServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVirtualDesktopNotificationService_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVirtualDesktopNotificationService_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVirtualDesktopNotificationService_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVirtualDesktopNotificationService_Register(This,pNotification,cookie)	\
    ( (This)->lpVtbl -> Register(This,pNotification,cookie) ) 

#define IVirtualDesktopNotificationService_Unregister(This,cookie)	\
    ( (This)->lpVtbl -> Unregister(This,cookie) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVirtualDesktopNotificationService_INTERFACE_DEFINED__ */



#ifndef __VirtualDesktopInternalTypes_LIBRARY_DEFINED__
#define __VirtualDesktopInternalTypes_LIBRARY_DEFINED__

/* library VirtualDesktopInternalTypes */
/* [uuid] */ 


EXTERN_C const IID LIBID_VirtualDesktopInternalTypes;

EXTERN_C const CLSID CLSID_CVirtualDesktopManagerInternal;

#ifdef __cplusplus

class DECLSPEC_UUID("c5e0cdca-7b6e-41b2-9fc4-d93975cc467b")
CVirtualDesktopManagerInternal;
#endif

EXTERN_C const CLSID CLSID_CVirtualDesktopNotificationService;

#ifdef __cplusplus

class DECLSPEC_UUID("a501fdec-4a09-464c-ae4e-1b9c21b84918")
CVirtualDesktopNotificationService;
#endif

EXTERN_C const CLSID CLSID_CImmersiveShell;

#ifdef __cplusplus

class DECLSPEC_UUID("c2f03a33-21f5-47fa-b4bb-156362a2f239")
CImmersiveShell;
#endif
#endif /* __VirtualDesktopInternalTypes_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


