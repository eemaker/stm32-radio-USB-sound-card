#include "bsp_audio.h"

extern I2S_HandleTypeDef hi2s3;

static void I2Sx_Init(uint32_t AudioFreq)
{
  /* Initialize the haudio_i2s Instance parameter */
  hi2s3.Instance = SPI3;

 /* Disable I2S block */
  __HAL_I2S_DISABLE(&hi2s3);

  hi2s3.Init.Mode = I2S_MODE_MASTER_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.AudioFreq = AudioFreq;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;

  if(HAL_I2S_GetState(&hi2s3) == HAL_I2S_STATE_RESET)
  {
    HAL_I2S_MspInit(&hi2s3);
  }
  /* Init the I2S */
  HAL_I2S_Init(&hi2s3);	

}
const uint32_t I2SFreq[8] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000};
const uint32_t I2SPLLN[8] = {256, 429, 213, 429, 426, 271, 258, 344};
const uint32_t I2SPLLR[8] = {5, 4, 4, 4, 4, 6, 3, 1};
uint8_t BSP_AUDIO_OUT_Init(uint8_t Volume, uint32_t AudioFreq)
{
	uint8_t index = 0, freqindex = 0xFF;
	RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;

	//get the according P,N value and set into config,this is for audio clock provide
	for(index = 0; index < 8; index++)
	{
		if(I2SFreq[index] == AudioFreq)
		{
			freqindex = index;
		}
	}
	HAL_RCCEx_GetPeriphCLKConfig(&RCC_ExCLKInitStruct);
	if(freqindex != 0xFF)
	{
		RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    		RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = I2SPLLN[freqindex];
    		RCC_ExCLKInitStruct.PLLI2S.PLLI2SR = I2SPLLR[freqindex];
    		HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
	}
	else
	{
		RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    		RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 258;
    		RCC_ExCLKInitStruct.PLLI2S.PLLI2SR = 3;
    		HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
	}

	/* I2S data transfer preparation:
	Prepare the Media to be used for the audio transfer from memory to I2S peripheral */
	/* Configure the I2S peripheral */
	I2Sx_Init(AudioFreq);
	
	// Enable DAC Power
	HAL_GPIO_WritePin(DAC_EN_GPIO_Port,DAC_EN_Pin,GPIO_PIN_SET);
	
	return AUDIO_OK;
}

uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option)
{
	/* Call the Media layer stop function */
	HAL_I2S_DMAStop(&hi2s3);

   if(Option == CODEC_PDWN_HW)
   {	
      // Disable DAC Power
      HAL_GPIO_WritePin(DAC_EN_GPIO_Port,DAC_EN_Pin,GPIO_PIN_RESET);
   }
	/* Return AUDIO_OK when all operations are correctly done */
	return AUDIO_OK;
}

uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
{
	/* Update the Media layer and enable it for play */
	HAL_I2S_Transmit_DMA(&hi2s3, pBuffer, DMA_MAX(Size/AUDIODATA_SIZE));
	return AUDIO_OK;
}

void BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size)
{
  HAL_I2S_Transmit_DMA(&hi2s3, pData, Size);
}

uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume)
{
	//if( 0 == Volume )HAL_GPIO_WritePin(AMP_EN_GPIO_Port,AMP_EN_Pin,GPIO_PIN_RESET);
	//else HAL_GPIO_WritePin(AMP_EN_GPIO_Port,AMP_EN_Pin,GPIO_PIN_SET);
	
	/* Return AUDIO_OK when all operations are correctly done */
	return AUDIO_OK;
}

uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd)
{
    return AUDIO_OK;
}

//-------------------for I2s interrupt--------------------------//
extern void TransferComplete_CallBack_FS(void);
extern void HalfTransfer_CallBack_FS(void);
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
  /* Manage the remaining file size and new address offset: This function
     should be coded by user (its prototype is already declared in stm324xg_eval_audio.h) */
  //BSP_AUDIO_OUT_TransferComplete_CallBack();
		TransferComplete_CallBack_FS();
}

/**
  * @brief Tx Transfer Half completed callbacks
  * @param hi2s: I2S handle
  */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
  /* Manage the remaining file size and new address offset: This function
     should be coded by user (its prototype is already declared in stm324xg_eval_audio.h) */
  //BSP_AUDIO_OUT_HalfTransfer_CallBack();
		HalfTransfer_CallBack_FS();
}

/**
  * @brief  I2S error callbacks.
  * @param  hi2s: I2S handle
  */
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
  //BSP_AUDIO_OUT_Error_CallBack();
}
