#ifndef __RENDERCHANNEL_H__
#define __RENDERCHANNEL_H__

#include <d3d9.h>
#include <d3dx9.h>

#include <process.h>
#include "../LibCore/CommandClient.h"
#include "../VideoGen/generator.h"

#ifndef Max_Obj_Cnt
#define Max_Obj_Cnt 20010
#endif



// this is for the render channel
// the render channel contains the self-reconstruct part, a video channel 

class RenderChannel{
public:
	static int				refCount;
	static HANDLE			presentMutex;  // all channels share one mutex
	cg::IDENTIFIER				taskId;
	char * rtspObject;  // the rtsp service name
	cg::VideoGen * generator;	

	cg::VideoItem * videoItem;
	IDirect3DDevice9 *		curDevice;
	cg::core::CommandClient *			cc;

	int						winWidth, winHeight;
	int						frameIndex; // the frame index in the group of renders for current channel
	//HANDLE					presentMutex;  // setted from outside
	HANDLE					presentEvent;  // 
	HANDLE					videoInitMutex; // video init mutex, synchronize with video server
	HANDLE					videoThread;
	DWORD					videoThreadId;
	float					gap;
	float					vs_data[10000];
	UINT					arr[4];
	char					up_buf[1000];
	string					gameName;
	bool					client_render;
	bool					windowCreated;

	// for all d3d context
	void*				device_list[Max_Obj_Cnt];
	void*				vb_list[Max_Obj_Cnt];
	void*				ib_list[Max_Obj_Cnt];
	void*				vd_list[Max_Obj_Cnt];
	void*				vs_list[Max_Obj_Cnt];
	void*				ps_list[Max_Obj_Cnt];
	void*				tex_list[Max_Obj_Cnt];
	void*				sb_list[Max_Obj_Cnt];
	void*				ctex_list[Max_Obj_Cnt];
	void*				chain_list[Max_Obj_Cnt];
	void*				surface_list[Max_Obj_Cnt];

	//inline void getDevice(){ curDevice = (LPDIRECT3DDEVICE9)(device_list[]) }

	BOOL				IsTextureFormatOk(D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat);
	DWORD				channelThreadId;
	HANDLE				channelThreadHandle;

	static DWORD WINAPI ChannelThreadProc(LPVOID param);
public:
	//IDirect3DDevice9
	int					op_code;
	int					obj_id;
	IDirect3D9 *		gD3d;
	D3DDISPLAYMODE		displayMode;
	D3DPRESENT_PARAMETERS d3dpp;
	HWND				hWnd;
	bool				useServerStyle;

	HANDLE				DeviceHandle;   // the event to notify the video encoder
	void				destroyD3DWindow();

	// create the d3d device, called when recving CreateDevice
	HRESULT				clientInit();
	HWND				initWindow(int widht, int height, DWORD dwExStyle, DWORD dwStyle);

	inline IDirect3DDevice9 * getDevice(int id){ curDevice = (LPDIRECT3DDEVICE9)(device_list[id]); return curDevice; }
	DWORD				startChannelThread();
	int					totalFunc;  // total functions in function array

	// constructor and destructor
	RenderChannel();
	~RenderChannel();

	bool				initRenderChannel(cg::IDENTIFIER cid, string name, SOCKET s);
	
	// getter and setters
	inline void			setPresentMutex(HANDLE p){ presentMutex = p; }
	inline HANDLE		getPresentMutex(){ return presentMutex; }
	inline void			setPresentEvent(HANDLE p){ presentEvent = p; }
	inline HANDLE		getPresentEvent(){ return presentEvent; }

	// rtsp
	int					initVideoGen();
	bool				enableRTSP, enableWriteToFile;
	inline bool			enableRTSPService(){ return enableRTSP; }
	inline bool			enableGenerateVideo(){ return enableWriteToFile; }
	void				onPresent(unsigned int tags);
	void				cleanUp();
	inline void			setEncoderOption(int option){ encoderOption = option; }
	inline int			getEncoderOption(){ return encoderOption; }


	int					imageWidth, imageHeight;
	int					encoderOption;   // 1 for x264, 2 for CUDA, 3 for NVENC, 4 for ADAPTIVE encoder
};


#endif   // __LIBRENDERCHANNEL_H__