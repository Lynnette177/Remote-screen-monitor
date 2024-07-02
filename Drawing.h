#pragma once
#ifndef DRAWING_H
#define DRAWING_H
#include "includes.h"
#include "global.h"
#include "dx11imageloader.h"
#include "UI.h"
class Drawing
{
private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;
	static bool bDraw;

public:
	static void Active();
	static bool isActive();
	static void Draw(ID3D11Device* pd3ddevice);
};

LPCSTR Drawing::lpWindowName = u8"Զ����Ļ���ϵͳ";
ImVec2 Drawing::vWindowSize = { 900, 600 };
ImGuiWindowFlags Drawing::WindowFlags = 0;
bool Drawing::bDraw = true;
bool FullWindow = false;
void* MainMonitoring = NULL;
void Drawing::Active()
{
	bDraw = true;
}

bool Drawing::isActive()
{
	return bDraw == true;
}

void Drawing::Draw(ID3D11Device* pd3ddevice)
{
	if (isActive())
	{
		static int client_number;
		ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
		{
			ImGui::Text(u8"����%d���ͻ��˽������ӡ�", client_number);
			client_number = 0;
			for (void* v : all_connected_clients) {
				
				ClientHandler* now_draw_client = (ClientHandler*)v;
				ImGui::Text(now_draw_client->client_info.c_str());
				if (!now_draw_client->generated_new_texture || !now_draw_client->off_line_pic_generated) {
					if (now_draw_client->off_line_pic_generated && !now_draw_client->generated_new_texture) {
						ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)now_draw_client->thumb_texture.GetTexture();
						now_draw_client->thumb_texture.Release_Texture();
						if (now_draw_client->thumb_texture.pDevice != pd3ddevice)
							now_draw_client->thumb_texture.pDevice = pd3ddevice;
						now_draw_client->thumb_texture.LoadTextureFromMemory(now_draw_client->data_buffer.data(), now_draw_client->data_buffer.size());
						now_draw_client->generated_new_texture = true;
					}
					else {
						ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)now_draw_client->thumb_texture.GetTexture();
						now_draw_client->thumb_texture.Release_Texture();
						if (now_draw_client->thumb_texture.pDevice != pd3ddevice)
							now_draw_client->thumb_texture.pDevice = pd3ddevice;
						now_draw_client->thumb_texture.LoadTextureFromMemory_To_Gray(now_draw_client->data_buffer.data(), now_draw_client->data_buffer.size());
						now_draw_client->off_line_pic_generated = true;
					}
				}
				client_number++;
				ImGui::Image(now_draw_client->thumb_texture.GetTexture(), ImVec2(now_draw_client->aspect_ratio * 100, 100)); // ����ͼƬ
				ImGui::SliderInt(u8"���֡��", &now_draw_client->frame_rate,1,100);
			}
			ImGui::Text(u8"ƽ��֡�� %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		ImGui::End();
	}

#ifdef _WINDLL
	if (GetAsyncKeyState(VK_INSERT) & 1)
		bDraw = !bDraw;
#endif
}

#endif
