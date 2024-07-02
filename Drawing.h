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
	static bool switched = true;
	if (isActive())
	{
		if (GetAsyncKeyState(VK_ESCAPE)) {
			FullWindow = false;
			reset_Frame_rate = true;
			switched = true;
		}
		static int client_number;
		if (!FullWindow) {
			if (switched) {
				ImGui::SetNextWindowSize(vWindowSize);
				ImGui::SetNextWindowBgAlpha(1.0f);
				switched = false;
			}
			ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
			{
				ImGui::Text(u8"����%d���ͻ��˽������ӡ�", client_number);
				client_number = 0;
				for (const auto& v : all_connected_clients) {
					ClientHandler* now_draw_client = (ClientHandler*)v;
					now_draw_client->image_lock.lock();
					if (reset_Frame_rate) {
						now_draw_client->frame_rate = 10;
					}
					if (!now_draw_client->generated_new_texture || !now_draw_client->off_line_pic_generated) {
						if (now_draw_client->off_line_pic_generated && !now_draw_client->generated_new_texture) {
							ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)now_draw_client->thumb_texture.GetTexture();
							now_draw_client->thumb_texture.Release_Texture();
							if (now_draw_client->thumb_texture.pDevice != pd3ddevice)
								now_draw_client->thumb_texture.pDevice = pd3ddevice;
							now_draw_client->thumb_texture.LoadTextureFromMemory(now_draw_client->image_data.data(), now_draw_client->image_data.size());
							now_draw_client->generated_new_texture = true;
						}
						else {
							ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)now_draw_client->thumb_texture.GetTexture();
							now_draw_client->thumb_texture.Release_Texture();
							if (now_draw_client->thumb_texture.pDevice != pd3ddevice)
								now_draw_client->thumb_texture.pDevice = pd3ddevice;
							now_draw_client->thumb_texture.LoadTextureFromMemory_To_Gray(now_draw_client->image_data.data(), now_draw_client->image_data.size());
							now_draw_client->off_line_pic_generated = true;
						}
					}
					client_number++;
					if (now_draw_client->online)
						ImGui::Text((now_draw_client->client_info + u8" ����").c_str());
					else
						ImGui::Text((now_draw_client->client_info + u8" ����").c_str());
					ImGui::Image(now_draw_client->thumb_texture.GetTexture(), ImVec2(now_draw_client->aspect_ratio * 100, 100)); // ����ͼƬ
					std::ostringstream oss,buttonlable;
					oss << u8"���֡�� " << client_number;
					buttonlable << u8"ȫ���ͻ�" << client_number;
					ImGui::SliderInt(oss.str().c_str(), &((ClientHandler*)v)->frame_rate, 1, 100);
					if (ImGui::Button(buttonlable.str().c_str())) {
						FullWindow = true;
						switched = true;
						MainMonitoring = (ClientHandler*)v;
					}
					now_draw_client->image_lock.unlock();
				}
				reset_Frame_rate = false;
				ImGui::Text(u8"ƽ��֡�� %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}
		}
		else {
			if (switched) {
				ImGui::SetNextWindowSize(ImVec2(client_width, client_height));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				switched = false;
			}
			ImGui::SetNextWindowBgAlpha(1.0f);
			ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
			{
				bool still_online = false;
				for (void* v : all_connected_clients) {
					((ClientHandler*)v)->image_lock.lock();
					if (v == (void* )MainMonitoring) {
						still_online = true;
						MainMonitoring->frame_rate = 30;
						MainMonitoring->main_monitoring = true;
						if ((!MainMonitoring->generated_new_texture || !MainMonitoring->off_line_pic_generated)) {
							if (MainMonitoring->off_line_pic_generated && !MainMonitoring->generated_new_texture) {
								ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)MainMonitoring->thumb_texture.GetTexture();
								MainMonitoring->thumb_texture.Release_Texture();
								if (MainMonitoring->thumb_texture.pDevice != pd3ddevice)
									MainMonitoring->thumb_texture.pDevice = pd3ddevice;
								MainMonitoring->thumb_texture.LoadTextureFromMemory(MainMonitoring->image_data.data(), MainMonitoring->image_data.size());
								MainMonitoring->generated_new_texture = true;
							}
							else {
								ID3D11ShaderResourceView* pSRV = (ID3D11ShaderResourceView*)MainMonitoring->thumb_texture.GetTexture();
								MainMonitoring->thumb_texture.Release_Texture();
								if (MainMonitoring->thumb_texture.pDevice != pd3ddevice)
									MainMonitoring->thumb_texture.pDevice = pd3ddevice;
								MainMonitoring->thumb_texture.LoadTextureFromMemory_To_Gray(MainMonitoring->image_data.data(), MainMonitoring->image_data.size());
								MainMonitoring->off_line_pic_generated = true;
							}
						}
						ImGui::Image(MainMonitoring->thumb_texture.GetTexture(), ImVec2(MainMonitoring->aspect_ratio * 1000, 1000));
						ImVec2 pos = ImGui::GetCursorScreenPos();
						if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
						{
							// ��ȡ���λ��
							ImVec2 mouse_pos = ImGui::GetMousePos();

							// ���������ͼƬ�ϵ����λ��
							int relative_x = mouse_pos.x - pos.x;
							int relative_y = 1000 - (pos.y - mouse_pos.y) + 5;//ָ��߶�
							MainMonitoring->command_lock.lock();
							if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						    	MainMonitoring->command = 1;
							else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
								MainMonitoring->command = 2;
							MainMonitoring->x = relative_x;
							MainMonitoring->y = relative_y;
							MainMonitoring->command_lock.unlock();
							// ���������꣨��������Ĵ�����ʹ�����ǣ�
							printf("Clicked at : (%d, %d)\n",  relative_x, relative_y);
						}
					}
					else ((ClientHandler*)v)->frame_rate = 1;
					((ClientHandler*)v)->image_lock.unlock();
				}
				if (!still_online) {
					MainMonitoring = NULL;
					FullWindow = false;
					switched = true;
					reset_Frame_rate = true;
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
