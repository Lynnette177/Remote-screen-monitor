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

LPCSTR Drawing::lpWindowName = u8"远程屏幕监控系统";
ImVec2 Drawing::vWindowSize = { 900, 600 };
ImGuiWindowFlags Drawing::WindowFlags = 0;
bool Drawing::bDraw = true;
bool FullWindow = false;
bool reset_Frame_rate = false;
ClientHandler* MainMonitoring = NULL;
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
		if (GetAsyncKeyState(VK_ESCAPE)) {
			FullWindow = false;
			reset_Frame_rate = true;
		}
		static int client_number;
		if (!FullWindow) {
		    ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
		    ImGui::SetNextWindowBgAlpha(1.0f);
			ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
			{
				ImGui::Text(u8"已有%d个客户端建立连接。", client_number);
				client_number = 0;
				for (void* v : all_connected_clients) {
					ClientHandler* now_draw_client = (ClientHandler*)v;
					if (reset_Frame_rate) {
						now_draw_client->frame_rate = 10;
					}
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
					if (now_draw_client->online)
						ImGui::Text((now_draw_client->client_info + u8" 在线").c_str());
					else
						ImGui::Text((now_draw_client->client_info + u8" 离线").c_str());
					ImGui::Image(now_draw_client->thumb_texture.GetTexture(), ImVec2(now_draw_client->aspect_ratio * 100, 100)); // 绘制图片
					ImGui::SliderInt(u8"监控帧率", &now_draw_client->frame_rate, 1, 100);
					if (ImGui::Button(u8"全屏")) {
						FullWindow = true;
						MainMonitoring = now_draw_client;
					}
				}
				reset_Frame_rate = false;
				ImGui::Text(u8"平均帧率 %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}
		}
		else {
			ImGui::SetNextWindowSize(ImVec2(client_width, client_height));
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowBgAlpha(1.0f);
			ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
			{
				bool still_online = false;
				for (void* v : all_connected_clients) {
					if (v == MainMonitoring) {
						still_online = true;
						MainMonitoring->frame_rate = 30;
						MainMonitoring->main_monitoring = true;
						if (!MainMonitoring->generated_new_texture || !MainMonitoring->off_line_pic_generated) {
							if (MainMonitoring->off_line_pic_generated && !MainMonitoring->generated_new_texture) {
								ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)MainMonitoring->thumb_texture.GetTexture();
								MainMonitoring->thumb_texture.Release_Texture();
								if (MainMonitoring->thumb_texture.pDevice != pd3ddevice)
									MainMonitoring->thumb_texture.pDevice = pd3ddevice;
								MainMonitoring->thumb_texture.LoadTextureFromMemory(MainMonitoring->data_buffer.data(), MainMonitoring->data_buffer.size());
								MainMonitoring->generated_new_texture = true;
							}
							else {
								ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)MainMonitoring->thumb_texture.GetTexture();
								MainMonitoring->thumb_texture.Release_Texture();
								if (MainMonitoring->thumb_texture.pDevice != pd3ddevice)
									MainMonitoring->thumb_texture.pDevice = pd3ddevice;
								MainMonitoring->thumb_texture.LoadTextureFromMemory_To_Gray(MainMonitoring->data_buffer.data(), MainMonitoring->data_buffer.size());
								MainMonitoring->off_line_pic_generated = true;
							}
						}
						ImGui::Image(MainMonitoring->thumb_texture.GetTexture(), ImVec2(MainMonitoring->aspect_ratio * 1000, 1000));
					}
					else MainMonitoring->frame_rate = 1;
				}
				if (!still_online) {
					MainMonitoring = NULL;
					FullWindow = false;
				}
			}
		}
		ImGui::End();
	}

#ifdef _WINDLL
	if (GetAsyncKeyState(VK_INSERT) & 1)
		bDraw = !bDraw;
#endif
}

#endif
