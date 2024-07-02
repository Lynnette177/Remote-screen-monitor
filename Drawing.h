#pragma once
#ifndef DRAWING_H
#define DRAWING_H
#include "includes.h"
#include "global.h"
#include "dx11imageloader.h"
#include "UI.h"
class Drawing
{//�����ࡣ������������size�Ȼ�����Ϣ
private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;
	static bool bDraw;//�Ƿ����ڻ��Ƶı�־��û��

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
	static uint64_t save_time = 0;
	static int save_interval = 100;
	static bool switched = true;
	if (isActive())
	{
		if (GetAsyncKeyState(VK_ESCAPE)) {//�����⵽esc�����£���ôҪ�л�����ȫ��ģʽ
			FullWindow = false;
			reset_Frame_rate = true;//��������֡�ʵı�־
			switched = true;
		}

		static int client_number;
		if (!FullWindow) {//�Ƿ���ȫ���ķ�ʽ��ʾ
			if (switched) {//�Ƿ���ȫ���л���ĵ�һ�Σ������Ҫ���贰�ڴ�С
				ImGui::SetNextWindowSize(vWindowSize);
				ImGui::SetNextWindowBgAlpha(1.0f);
				switched = false;
			}
			ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
			{
				int max_x = int(ImGui::GetWindowSize().x) / 300;
				ImGui::Text(u8"����%d���ͻ��˽������ӡ�", client_number);
				ImGui::SliderInt(u8"�Զ�����ͼƬʱ����(��)", &save_interval, 10, 120);
				client_number = 0;
				bool did_save = false;
				int row = 0;
				int column = 0;
				for (const auto& v : all_connected_clients) {//�������пͻ��ˣ��ֱ����
					ImGui::SetCursorPos(ImVec2(column * 300 + 10, 100 + row * 240));
					column++;
					if (column >= max_x) {
						column = 0;
						row++;
					}
					ClientHandler* now_draw_client = (ClientHandler*)v;
					ImGui::BeginChild(std::to_string(client_number).c_str(), ImVec2(300, 240), true);

					std::lock_guard<std::mutex> lock(now_draw_client->image_lock); // �Զ�������
					if (reset_Frame_rate) {
						now_draw_client->frame_rate = 10;
					}
					//����Ѿ��Ӷ��������ݼ��س�������������Ҫ���½��м��ء�����Ҫ��vector�е�uint8_t���ݼ�������
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
							//��������ߣ���ôҪ��ͼƬ���سɻ�ɫ�ġ���ͬ�ļ��غ����������ֶ�����
							now_draw_client->thumb_texture.LoadTextureFromMemory_To_Gray(now_draw_client->image_data.data(), now_draw_client->image_data.size());
							now_draw_client->off_line_pic_generated = true;
						}
						if (((ClientHandler*)v)->able_to_save) {
							//������Ա���ı�־���棬�ͽ��б��棬����Ϊ��
							saveVectorToBinaryFile(now_draw_client->image_data, now_draw_client->id , getCurrentTime() + ".jpeg");
							((ClientHandler*)v)->able_to_save = false; //��Ϊ�� ֪ͨ�ͻ����Ѿ�������ˣ���һ�β����ٴ��͸߷ֱ���ͼƬ��
						}
					}
					client_number++;
					ImGui::Text((now_draw_client->id).c_str());
					if (now_draw_client->online)
						ImGui::Text((now_draw_client->client_info + u8" ����").c_str());
					else
						ImGui::Text((now_draw_client->client_info + u8" ����").c_str());
					ImGui::Image(now_draw_client->thumb_texture.GetTexture(), ImVec2(now_draw_client->aspect_ratio * 100, 100)); // ����ͼƬ
					std::ostringstream oss, buttonlable, tickbox;
					oss << u8"���֡�� " << client_number;
					buttonlable << u8"ȫ���ͻ�" << client_number;
					tickbox << u8"�Զ������ͼ" << client_number;
					ImGui::SliderInt(oss.str().c_str(), &((ClientHandler*)v)->frame_rate, 1, 100);
					if (ImGui::Button(buttonlable.str().c_str())) {
						FullWindow = true;
						switched = true;
						MainMonitoring = (ClientHandler*)v;
					}
					ImGui::SameLine();
					//����Ƿ��û������CheckBox
					bool tmp_tick = ((ClientHandler*)v)->save_image;
					ImGui::Checkbox(tickbox.str().c_str(), &((ClientHandler*)v)->save_image);
					if (!tmp_tick == ((ClientHandler*)v)->save_image) save_time = 0;//���������ۼ�ʱ������ ��������ͼƬ���¿�ʼ��ʱ
					((ClientHandler*)v)->main_monitoring = ((ClientHandler*)v)->save_image;//���ȷ��Ҫ����������Ϊ��֪ͨ�ͻ���Ҫ��ȡ�߷ֱ��ʵ�ͼƬ����Ϊ��һ�μ���ͼƬ��ʱ��Ҫ������
					if (((ClientHandler*)v)->save_image && GetTickCount64() - save_time > save_interval * 1000) {
						((ClientHandler*)v)->able_to_save = true;//ֻ�е�ѡ�񱣴���������Ҽ�ʱ�Ѿ������û�����ֵ�����ʶλ��Ϊ��
						did_save = true;//������ѭ�������ü�ʱ
					}

					ImGui::EndChild();
				}
				reset_Frame_rate = false;
				if (did_save)
				    save_time = GetTickCount64();
				ImGui::Text(u8"ƽ��֡�� %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}
		}
		else {
			if (switched) {//ͬ�� �л���־
				ImGui::SetNextWindowSize(ImVec2(client_width, client_height));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowBgAlpha(1.0f);
				switched = false;
			}
			ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
			{
				bool still_online = false;
				for (void* v : all_connected_clients) {
					std::lock_guard<std::mutex> lock(((ClientHandler*)v)->image_lock); // �Զ�������
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
							// ����������� ������ʵ���� ���͸��ͻ��ˣ�׼�����
							//printf("Clicked at : (%d, %d)\n",  relative_x, relative_y);
						}
					}
					else ((ClientHandler*)v)->frame_rate = 1;//����������Ҫ�۲��ߵĻ�����ȫ����Ϊ֡Ϊ1��������Դռ��
				}
				if (!still_online) {//�������ʱ�䳬ʱ�����Զ��л��ط�ȫ��ģʽ
					MainMonitoring = NULL;
					FullWindow = false;
					switched = true;
					reset_Frame_rate = true;
				}
			}
		}
		ImGui::End();
	}
}

#endif
