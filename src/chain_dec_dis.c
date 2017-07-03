#include "mcfw/src_linux/mcfw_api/usecases/multich_common.h"
#include "mcfw/src_linux/mcfw_api/usecases/multich_ipcbits.h"
#include "mcfw/interfaces/link_api/system_tiler.h"

#define NUM_CH 1

static UInt32 DSP_LINK_ID_ALG;
static UInt32 DSP_LINK_ID_IPC_FRAME_IN;
static UInt32 VPSS_LINK_ID_IPC_FRAME_OUT;


#define     MULTICH_NUM_SWMS_MAX_BUFFERS              (7)
#define     MAX_BUFFERING_QUEUE_LEN_PER_CH           (50)
#define     BIT_BUF_LENGTH_LIMIT_FACTOR_HD            (5)

static SystemVideo_Ivahd2ChMap_Tbl systemVid_encDecIvaChMapTbl =
{
    .isPopulated = 1,
    .ivaMap[0] =
    {
        .EncNumCh  = 4,
        .EncChList = {0, 1, 4, 5},

        .DecNumCh  = 4,
        .DecChList = {0, 4, 5, 6},
    },

    .ivaMap[1] =
    {
        .EncNumCh  = 4,
        .EncChList = {2, 6, 7, 8},

        .DecNumCh  = 4,
        .DecChList = {1, 2, 7, 8},
    },

    .ivaMap[2] =
    {
        .EncNumCh  = 4,
        .EncChList = {3, 9, 10, 11},

        .DecNumCh  = 4,
        .DecChList = {3, 9, 10, 11},
    },
};

typedef struct {
    UInt32 ipcOutVideoId;
    UInt32 ipcInVpssId;
} MultiCh_VdecVdisObj;

MultiCh_VdecVdisObj gMultiCh_VdecVdisObj;

void chain_dec_dis_create(int ch_num) {
    IpcBitsOutLinkHLOS_CreateParams   ipcBitsOutHostPrm;
    IpcBitsInLinkRTOS_CreateParams    ipcBitsInVideoPrm;
    DecLink_CreateParams        decPrm;
    IpcLink_CreateParams        ipcOutVideoPrm;
    IpcLink_CreateParams        ipcInVpssPrm;
    DupLink_CreateParams        dupPrm;
    static SwMsLink_CreateParams       swMsPrm[VDIS_DEV_MAX];
    DisplayLink_CreateParams    displayPrm[VDIS_DEV_MAX];
    IpcFramesOutLinkRTOS_CreateParams ipcFramesOutVpssPrm;
    IpcFramesInLinkRTOS_CreateParams  ipcFramesInDspPrm;
    AlgLink_CreateParams              algPrms;

    IpcLink_CreateParams        ipcOutVpssPrm;
    IpcLink_CreateParams        ipcInVideoPrm;
    EncLink_CreateParams        encPrm;
    IpcBitsOutLinkRTOS_CreateParams   ipcBitsOutVideoPrm;
    IpcBitsInLinkHLOS_CreateParams    ipcBitsInHostPrm;

    UInt32 i;
    UInt32 dupId;
    

    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVpssPrm);
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVideoPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkHLOS_CreateParams,ipcBitsOutHostPrm);
    MULTICH_INIT_STRUCT(IpcBitsInLinkRTOS_CreateParams,ipcBitsInVideoPrm);
    MULTICH_INIT_STRUCT(DecLink_CreateParams, decPrm);
    MULTICH_INIT_STRUCT(IpcFramesOutLinkRTOS_CreateParams,ipcFramesOutVpssPrm);
    MULTICH_INIT_STRUCT(IpcFramesInLinkRTOS_CreateParams,ipcFramesInDspPrm);
    MULTICH_INIT_STRUCT(AlgLink_CreateParams, algPrms);
    for (i = 0; i < VDIS_DEV_MAX;i++) {
        MULTICH_INIT_STRUCT(DisplayLink_CreateParams,displayPrm[i]);
        MULTICH_INIT_STRUCT(SwMsLink_CreateParams ,swMsPrm[i]);
    }

    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcOutVpssPrm);
    MULTICH_INIT_STRUCT(IpcLink_CreateParams,ipcInVideoPrm);
    MULTICH_INIT_STRUCT(EncLink_CreateParams, encPrm);
    MULTICH_INIT_STRUCT(IpcBitsOutLinkRTOS_CreateParams,ipcBitsOutVideoPrm);
    MULTICH_INIT_STRUCT(IpcBitsInLinkHLOS_CreateParams,ipcBitsInHostPrm);
    

    MultiCh_detectBoard();

    /* reset for display */
    System_linkControl( SYSTEM_LINK_ID_M3VPSS, SYSTEM_M3VPSS_CMD_RESET_VIDEO_DEVICES, NULL, 0, TRUE);

    /* set decode channel map */
    System_linkControl( SYSTEM_LINK_ID_M3VIDEO, SYSTEM_COMMON_CMD_SET_CH2IVAHD_MAP_TBL, &systemVid_encDecIvaChMapTbl, sizeof(SystemVideo_Ivahd2ChMap_Tbl), TRUE);

    SystemTiler_disableAllocator();

    /* link used */
    gVdecModuleContext.ipcBitsOutHLOSId = SYSTEM_HOST_LINK_ID_IPC_BITS_OUT_0;
    gVdecModuleContext.ipcBitsInRTOSId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_IN_0;
    gVdecModuleContext.decId            = SYSTEM_LINK_ID_VDEC_0;

    gMultiCh_VdecVdisObj.ipcOutVideoId  = SYSTEM_VIDEO_LINK_ID_IPC_OUT_M3_0;
    gMultiCh_VdecVdisObj.ipcInVpssId    = SYSTEM_VPSS_LINK_ID_IPC_IN_M3_0;

    gVdisModuleContext.swMsId[0]        = SYSTEM_LINK_ID_SW_MS_MULTI_INST_0;
    gVdisModuleContext.displayId[0]     = SYSTEM_LINK_ID_DISPLAY_0; // ON AND OFF CHIP HDMI

    DSP_LINK_ID_ALG                 = SYSTEM_LINK_ID_ALG_0;
    DSP_LINK_ID_IPC_FRAME_IN        = SYSTEM_DSP_LINK_ID_IPC_FRAMES_IN_0;
    VPSS_LINK_ID_IPC_FRAME_OUT      = SYSTEM_VPSS_LINK_ID_IPC_FRAMES_OUT_0;
    dupId                           = SYSTEM_VPSS_LINK_ID_DUP_0;

    gVcapModuleContext.deiId[0]         = SYSTEM_LINK_ID_DEI_0;
    gVencModuleContext.encId        = SYSTEM_LINK_ID_VENC_0;
    gVencModuleContext.ipcBitsOutRTOSId  = SYSTEM_VIDEO_LINK_ID_IPC_BITS_OUT_0;
    gVencModuleContext.ipcBitsInHLOSId   = SYSTEM_HOST_LINK_ID_IPC_BITS_IN_0;
    UInt32 ipcOutVpssId = SYSTEM_VPSS_LINK_ID_IPC_OUT_M3_0;
    UInt32 ipcInVideoId = SYSTEM_VIDEO_LINK_ID_IPC_IN_M3_0;


    /* ipc bits out host link */
    ipcBitsOutHostPrm.baseCreateParams.outQueParams[0].nextLink= gVdecModuleContext.ipcBitsInRTOSId;
    ipcBitsOutHostPrm.baseCreateParams.notifyNextLink       = FALSE;
    ipcBitsOutHostPrm.baseCreateParams.notifyPrevLink       = FALSE;
    ipcBitsOutHostPrm.baseCreateParams.noNotifyMode         = TRUE;
    ipcBitsOutHostPrm.baseCreateParams.numOutQue            = 1;
    ipcBitsOutHostPrm.inQueInfo.numCh                       = gVdecModuleContext.vdecConfig.numChn;

    for (i=0; i<ipcBitsOutHostPrm.inQueInfo.numCh; i++)
    {
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].width = gVdecModuleContext.vdecConfig.decChannelParams[i].maxVideoWidth;

        ipcBitsOutHostPrm.inQueInfo.chInfo[i].height = gVdecModuleContext.vdecConfig.decChannelParams[i].maxVideoHeight;

        ipcBitsOutHostPrm.inQueInfo.chInfo[i].scanFormat = SYSTEM_SF_PROGRESSIVE;

        ipcBitsOutHostPrm.inQueInfo.chInfo[i].bufType        = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].codingformat   = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].dataFormat     = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].memType        = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].startX         = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].startY         = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].pitch[0]       = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].pitch[1]       = 0; // NOT USED
        ipcBitsOutHostPrm.inQueInfo.chInfo[i].pitch[2]       = 0; // NOT USED

        ipcBitsOutHostPrm.maxQueueDepth[i] = MAX_BUFFERING_QUEUE_LEN_PER_CH;
        ipcBitsOutHostPrm.chMaxReqBufSize[i] = (ipcBitsOutHostPrm.inQueInfo.chInfo[i].width * ipcBitsOutHostPrm.inQueInfo.chInfo[i].height); 
        ipcBitsOutHostPrm.totalBitStreamBufferSize [i] = (ipcBitsOutHostPrm.chMaxReqBufSize[i] * BIT_BUF_LENGTH_LIMIT_FACTOR_HD);

    }

    /* ipc bits in video link */
    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkId    = gVdecModuleContext.ipcBitsOutHLOSId;
    ipcBitsInVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcBitsInVideoPrm.baseCreateParams.outQueParams[0].nextLink  = gVdecModuleContext.decId;
    ipcBitsInVideoPrm.baseCreateParams.noNotifyMode              = TRUE;
    ipcBitsInVideoPrm.baseCreateParams.notifyNextLink            = TRUE;
    ipcBitsInVideoPrm.baseCreateParams.notifyPrevLink            = FALSE;
    ipcBitsInVideoPrm.baseCreateParams.numOutQue                 = 1;

    /* decode link */
    for (i=0; i<ipcBitsOutHostPrm.inQueInfo.numCh; i++) {
        if(gVdecModuleContext.vdecConfig.decChannelParams[i].isCodec == VDEC_CHN_H264)
            decPrm.chCreateParams[i].format                 = IVIDEO_H264HP;
        else if(gVdecModuleContext.vdecConfig.decChannelParams[i].isCodec == VDEC_CHN_MPEG4)
            decPrm.chCreateParams[i].format                 = IVIDEO_MPEG4ASP;
        else if(gVdecModuleContext.vdecConfig.decChannelParams[i].isCodec == VDEC_CHN_MJPEG)
            decPrm.chCreateParams[i].format                 = IVIDEO_MJPEG;
        else if(gVdecModuleContext.vdecConfig.decChannelParams[i].isCodec == VDEC_CHN_MPEG2)
            decPrm.chCreateParams[i].format                 = IVIDEO_MPEG2HP;

        decPrm.chCreateParams[i].numBufPerCh = gVdecModuleContext.vdecConfig.decChannelParams[i].numBufPerCh;
        decPrm.chCreateParams[i].profile                = IH264VDEC_PROFILE_ANY;
        decPrm.chCreateParams[i].displayDelay = gVdecModuleContext.vdecConfig.decChannelParams[i].displayDelay;
        decPrm.chCreateParams[i].dpbBufSizeInFrames = IH264VDEC_DPB_NUMFRAMES_AUTO;
        if (gVdecModuleContext.vdecConfig.decChannelParams[i].fieldPicture) {
            OSA_printf("MULTICH_VDEC_VDIS:INFO ChId[%d] configured for field picture\n",i);
            decPrm.chCreateParams[i].processCallLevel   = VDEC_FIELDLEVELPROCESSCALL;
        }
        else {
            decPrm.chCreateParams[i].processCallLevel   = VDEC_FRAMELEVELPROCESSCALL;
        }
        decPrm.chCreateParams[i].targetMaxWidth  = ipcBitsOutHostPrm.inQueInfo.chInfo[i].width;
        decPrm.chCreateParams[i].targetMaxHeight = ipcBitsOutHostPrm.inQueInfo.chInfo[i].height;
        decPrm.chCreateParams[i].defaultDynamicParams.targetFrameRate = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.frameRate;
        decPrm.chCreateParams[i].defaultDynamicParams.targetBitRate = gVdecModuleContext.vdecConfig.decChannelParams[i].dynamicParam.targetBitRate;
        decPrm.chCreateParams[i].tilerEnable = FALSE;
        decPrm.chCreateParams[i].enableWaterMarking = gVdecModuleContext.vdecConfig.decChannelParams[i].enableWaterMarking;
    }

    decPrm.inQueParams.prevLinkId       = gVdecModuleContext.ipcBitsInRTOSId;
    decPrm.inQueParams.prevLinkQueId    = 0;
    decPrm.outQueParams.nextLink        = gMultiCh_VdecVdisObj.ipcOutVideoId;

    /* ipc out video link */
    ipcOutVideoPrm.inQueParams.prevLinkId    = gVdecModuleContext.decId;
    ipcOutVideoPrm.inQueParams.prevLinkQueId = 0;
    ipcOutVideoPrm.outQueParams[0].nextLink  = gMultiCh_VdecVdisObj.ipcInVpssId;
    ipcOutVideoPrm.notifyNextLink            = TRUE;
    ipcOutVideoPrm.notifyPrevLink            = TRUE;
    ipcOutVideoPrm.numOutQue                 = 1;

    /* ipc in vpss link */
    ipcInVpssPrm.inQueParams.prevLinkId    = gMultiCh_VdecVdisObj.ipcOutVideoId;
    ipcInVpssPrm.inQueParams.prevLinkQueId = 0;
    ipcInVpssPrm.notifyNextLink            = TRUE;
    ipcInVpssPrm.notifyPrevLink            = TRUE;
    ipcInVpssPrm.numOutQue                 = 1;
    //ipcInVpssPrm.outQueParams[0].nextLink   =  gVdisModuleContext.swMsId[0];
    ipcInVpssPrm.outQueParams[0].nextLink   =  VPSS_LINK_ID_IPC_FRAME_OUT;

    /* OSD */
    //ipcFramesOutVpssPrm.baseCreateParams.noNotifyMode = TRUE;
    ipcFramesOutVpssPrm.baseCreateParams.noNotifyMode = FALSE;
    ipcFramesOutVpssPrm.baseCreateParams.notifyPrevLink = TRUE;
    ipcFramesOutVpssPrm.baseCreateParams.notifyNextLink = TRUE;
    ipcFramesOutVpssPrm.baseCreateParams.inQueParams.prevLinkId = gMultiCh_VdecVdisObj.ipcInVpssId;
    ipcFramesOutVpssPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcFramesOutVpssPrm.baseCreateParams.numOutQue = 1;
    ipcFramesOutVpssPrm.baseCreateParams.outQueParams[0].nextLink =  dupId;;
    ipcFramesOutVpssPrm.baseCreateParams.processLink = DSP_LINK_ID_IPC_FRAME_IN;
    ipcFramesOutVpssPrm.baseCreateParams.notifyProcessLink = TRUE;

    //ipcFramesInDspPrm.baseCreateParams.noNotifyMode   = TRUE;
    ipcFramesInDspPrm.baseCreateParams.noNotifyMode   = FALSE;
    ipcFramesInDspPrm.baseCreateParams.notifyPrevLink = TRUE;
    ipcFramesInDspPrm.baseCreateParams.notifyNextLink = TRUE;
    ipcFramesInDspPrm.baseCreateParams.inQueParams.prevLinkId = VPSS_LINK_ID_IPC_FRAME_OUT;
    ipcFramesInDspPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcFramesInDspPrm.baseCreateParams.numOutQue   = 1;
    ipcFramesInDspPrm.baseCreateParams.outQueParams[0].nextLink = DSP_LINK_ID_ALG;

    algPrms.inQueParams.prevLinkId = DSP_LINK_ID_IPC_FRAME_IN;
    algPrms.inQueParams.prevLinkQueId = 0;
    algPrms.enableOSDAlg = TRUE;
    algPrms.osdChCreateParams[0].maxWidth = 500;
    algPrms.osdChCreateParams[0].maxHeight = 300;

    dupPrm.inQueParams.prevLinkId         = VPSS_LINK_ID_IPC_FRAME_OUT;
    dupPrm.inQueParams.prevLinkQueId      = 0;
    dupPrm.numOutQue                      = 2;
    dupPrm.outQueParams[0].nextLink       = ipcOutVpssId;
    dupPrm.outQueParams[1].nextLink       = gVdisModuleContext.swMsId[0];
    dupPrm.notifyNextLink                 = TRUE;


    /* sw mosaic link */
    swMsPrm[0].numSwMsInst = 1;
    swMsPrm[0].swMsInstId[0]        = SYSTEM_SW_MS_SC_INST_DEI_SC_NO_DEI;
    swMsPrm[0].swMsInstStartWin[0]  = 0;
    swMsPrm[0].swMsInstStartWin[1]  = 10;
    swMsPrm[0].enableProcessTieWithDisplay = TRUE;
    swMsPrm[0].includeVipScInDrvPath = FALSE;

    VDIS_DEV vdDevId = VDIS_DEV_HDMI;

    swMsPrm[0].inQueParams.prevLinkId     = dupId;
    swMsPrm[0].inQueParams.prevLinkQueId  = 1;
    swMsPrm[0].outQueParams.nextLink      = gVdisModuleContext.displayId[0];
    swMsPrm[0].numOutBuf                  = MULTICH_NUM_SWMS_MAX_BUFFERS;
    swMsPrm[0].maxInputQueLen             = SYSTEM_SW_MS_INVALID_INPUT_QUE_LEN;
    swMsPrm[0].maxOutRes                  = VSYS_STD_1080P_60;
    swMsPrm[0].initOutRes                 = gVdisModuleContext.vdisConfig.deviceParams[VDIS_DEV_HDMI].resolution;
    swMsPrm[0].lineSkipMode               = FALSE;
    swMsPrm[0].enableLayoutGridDraw       = gVdisModuleContext.vdisConfig.enableLayoutGridDraw;

    MultiCh_swMsGetDefaultLayoutPrm(vdDevId, &swMsPrm[0], FALSE);    /* both from 0-16 chnl */

    /* display link */
    displayPrm[0].inQueParams[0].prevLinkId    = gVdisModuleContext.swMsId[0];
    displayPrm[0].inQueParams[0].prevLinkQueId = 0;
    displayPrm[0].displayRes                   = VSYS_STD_1080P_60;
    displayPrm[0].numInputQueues               = 1;

    /* Add encode ## */  
    /* IPC Out VPSSS M3 link */
    ipcOutVpssPrm.inQueParams.prevLinkId       = dupId; 
    ipcOutVpssPrm.inQueParams.prevLinkQueId    = 0;
    ipcOutVpssPrm.numOutQue = 1;
    ipcOutVpssPrm.outQueParams[0].nextLink     = ipcInVideoId;
    ipcOutVpssPrm.notifyNextLink               = TRUE;
    ipcOutVpssPrm.notifyPrevLink               = TRUE;
    ipcOutVpssPrm.noNotifyMode                 = FALSE;

    /* IPC In Video M3 link */
    ipcInVideoPrm.inQueParams.prevLinkId       = ipcOutVpssId;
    ipcInVideoPrm.inQueParams.prevLinkQueId    = 0;
    ipcInVideoPrm.numOutQue                    = 1;
    ipcInVideoPrm.outQueParams[0].nextLink     = gVencModuleContext.encId;
    ipcInVideoPrm.notifyNextLink               = TRUE;
    ipcInVideoPrm.notifyPrevLink               = TRUE;
    ipcInVideoPrm.noNotifyMode                 = FALSE;


    /* Encode link */
    EncLink_CreateParams_Init(&encPrm);
    encPrm.numBufPerCh[0] = 4;
    for (i = 0; i < ch_num; i++) {
        EncLink_ChCreateParams *pLinkChPrm  = &encPrm.chCreateParams[i];

        VENC_CHN_PARAMS_S *pChPrm           = &gVencModuleContext.vencConfig.encChannelParams[i];
        pLinkChPrm->format                  = IVIDEO_H264HP;
        pLinkChPrm->profile                 = gVencModuleContext.vencConfig.h264Profile[i];
        pLinkChPrm->dataLayout              = VCODEC_FIELD_SEPARATED;
        pLinkChPrm->fieldMergeEncodeEnable  = FALSE;
        pLinkChPrm->enableAnalyticinfo      = pChPrm->enableAnalyticinfo;
        pLinkChPrm->enableWaterMarking      = pChPrm->enableWaterMarking;
        pLinkChPrm->maxBitRate              = pChPrm->maxBitRate;
        pLinkChPrm->encodingPreset          = pChPrm->encodingPreset;
        pLinkChPrm->rateControlPreset       = pChPrm->rcType;
        pLinkChPrm->enableSVCExtensionFlag  = pChPrm->enableSVCExtensionFlag;
        pLinkChPrm->numTemporalLayer        = pChPrm->numTemporalLayer;

        EncLink_ChDynamicParams *pLinkDynPrm = &pLinkChPrm->defaultDynamicParams;
        VENC_CHN_DYNAMIC_PARAM_S *pDynPrm    = &pChPrm->dynamicParam;
        pLinkDynPrm->intraFrameInterval     = pDynPrm->intraFrameInterval;
        pLinkDynPrm->targetBitRate          = pDynPrm->targetBitRate;
        pLinkDynPrm->interFrameInterval     = 1;
        pLinkDynPrm->mvAccuracy             = IVIDENC2_MOTIONVECTOR_QUARTERPEL;
        pLinkDynPrm->inputFrameRate         = pDynPrm->inputFrameRate;
        pLinkDynPrm->rcAlg                  = pDynPrm->rcAlg;
        pLinkDynPrm->qpMin                  = pDynPrm->qpMin;
        pLinkDynPrm->qpMax                  = pDynPrm->qpMax;
        pLinkDynPrm->qpInit                 = pDynPrm->qpInit;
        pLinkDynPrm->vbrDuration            = pDynPrm->vbrDuration;
        pLinkDynPrm->vbrSensitivity         = pDynPrm->vbrSensitivity;
    }
    encPrm.inQueParams.prevLinkId    = ipcInVideoId;
    encPrm.inQueParams.prevLinkQueId = 0;
    encPrm.outQueParams.nextLink     = gVencModuleContext.ipcBitsOutRTOSId;

    /* IPC Bits Out Video M3 link */
    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkId    = gVencModuleContext.encId;
    ipcBitsOutVideoPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    ipcBitsOutVideoPrm.baseCreateParams.numOutQue                 = 1;
    ipcBitsOutVideoPrm.baseCreateParams.outQueParams[0].nextLink  = gVencModuleContext.ipcBitsInHLOSId;
    MultiCh_ipcBitsInitCreateParams_BitsOutRTOS(&ipcBitsOutVideoPrm, TRUE);

    /* IPC Bits In Host link */
    ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkId    = gVencModuleContext.ipcBitsOutRTOSId;
    ipcBitsInHostPrm.baseCreateParams.inQueParams.prevLinkQueId = 0;
    MultiCh_ipcBitsInitCreateParams_BitsInHLOS(&ipcBitsInHostPrm);

    /* create link */
    System_linkCreate(gVdecModuleContext.ipcBitsOutHLOSId,&ipcBitsOutHostPrm,sizeof(ipcBitsOutHostPrm));
    System_linkCreate(gVdecModuleContext.ipcBitsInRTOSId,&ipcBitsInVideoPrm,sizeof(ipcBitsInVideoPrm));
    System_linkCreate(gVdecModuleContext.decId, &decPrm, sizeof(decPrm));

    System_linkCreate(gMultiCh_VdecVdisObj.ipcOutVideoId, &ipcOutVideoPrm, sizeof(ipcOutVideoPrm));
    System_linkCreate(gMultiCh_VdecVdisObj.ipcInVpssId  , &ipcInVpssPrm, sizeof(ipcInVpssPrm));

    System_linkCreate(VPSS_LINK_ID_IPC_FRAME_OUT, &ipcFramesOutVpssPrm, sizeof(ipcFramesOutVpssPrm));
    System_linkCreate(DSP_LINK_ID_IPC_FRAME_IN, &ipcFramesInDspPrm, sizeof(ipcFramesInDspPrm));
    System_linkCreate(DSP_LINK_ID_ALG, &algPrms, sizeof(algPrms));
    System_linkCreate(dupId, &dupPrm    , sizeof(dupPrm));

    System_linkCreate(ipcOutVpssId, &ipcOutVpssPrm, sizeof(ipcOutVpssPrm));
    System_linkCreate(ipcInVideoId, &ipcInVideoPrm, sizeof(ipcInVideoPrm));
    System_linkCreate(gVencModuleContext.encId, &encPrm, sizeof(encPrm));
    System_linkCreate(gVencModuleContext.ipcBitsOutRTOSId, &ipcBitsOutVideoPrm, sizeof(ipcBitsOutVideoPrm));
    System_linkCreate(gVencModuleContext.ipcBitsInHLOSId, &ipcBitsInHostPrm, sizeof(ipcBitsInHostPrm));

    System_linkCreate(gVdisModuleContext.swMsId[0]  , &swMsPrm[0], sizeof(swMsPrm[0]));
    System_linkCreate(gVdisModuleContext.displayId[0], &displayPrm[0], sizeof(displayPrm[0]));

   
}

void chain_dec_dis_1080p_3x3_layout(VDIS_MOSAIC_S * prm_mosaic) {
    UInt32 width = 1920, height = 1080;
    UInt32 row_max = 3, col_max = 3;

    prm_mosaic->numberOfWindows       = row_max * col_max;
    prm_mosaic->onlyCh2WinMapChanged  = FALSE;
    prm_mosaic->displayWindow.height  = height;
    prm_mosaic->displayWindow.width   = width;
    prm_mosaic->displayWindow.start_X = 0;
    prm_mosaic->displayWindow.start_Y = 0;

    UInt32 i;
    for (i = 0; i < VDIS_MOSAIC_WIN_MAX; i++) 
        prm_mosaic->chnMap[i] = VDIS_CHN_INVALID;

    UInt32 win;
    UInt32 row, col;
    for(row = 0; row < row_max; row++) {
        for(col = 0; col < col_max; col++) {
            win = row * col_max + col;

            prm_mosaic->winList[win].width = VsysUtils_floor(width / col_max, 16);
            prm_mosaic->winList[win].height = VsysUtils_floor(height / row_max, 1);
            prm_mosaic->winList[win].start_X = prm_mosaic->winList[win].width * col;
            prm_mosaic->winList[win].start_Y = prm_mosaic->winList[win].height * row;

            prm_mosaic->chnMap[win] = win;
            prm_mosaic->useLowCostScaling[win] = 0;
        }
    }
}


void chain_dec_dis_delete() {
    UInt32 dupId;
    dupId        = SYSTEM_VPSS_LINK_ID_DUP_0;

    Vdec_delete();
    Vdis_delete();;
    System_linkDelete(gVdisModuleContext.swMsId[0]);
    System_linkDelete(dupId);
    System_linkDelete(DSP_LINK_ID_ALG);
    System_linkDelete(DSP_LINK_ID_IPC_FRAME_IN);
    System_linkDelete(VPSS_LINK_ID_IPC_FRAME_OUT);
    System_linkDelete(gMultiCh_VdecVdisObj.ipcInVpssId );
    System_linkDelete(gMultiCh_VdecVdisObj.ipcOutVideoId );
   
    MultiCh_prfLoadCalcEnable(FALSE, TRUE, FALSE);

    SystemTiler_enableAllocator();
}


