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
ImVec2 Drawing::vWindowSize = { 1500, 700 };
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

void Render_History(ClientHandler* client, const std::string& id, bool * show_history, ID3D11Device* pd3ddevice) {
	static ImVec2 history_windows_location = {};
	static const std::regex pattern(R"((\d{4})-(\d{2})-(\d{2}) (\d{2})_(\d{2})_(\d{2})\.jpeg)");
	static ImVec2 full_screen_pic_size = {};
	static Texture Clicked_Full_screen_image;
	static bool show_full_screen = false;
	static bool render_video = false;
	static std::string show_full_screen_name;

	ImGui::SetNextWindowSize(ImVec2(1200,900),ImGuiCond_Once);
	ImGui::Begin((id + u8"����ʷͼƬ").c_str(), show_history, 0);
	std::string rpath = Get_path_by_id(id);
	if (!fs::exists(rpath)) {
		ImGui::Text(u8"û���κ���ʷ��¼\n�������ŵ����¼��ʷ�İ�ť��������");
		ImGui::End();
		return;
	}
	int pic_count = 0;
	try {
		for (const auto& entry : fs::directory_iterator(rpath)) {
			if (fs::is_regular_file(entry.path())) { // ֻ������ͨ�ļ�
				if (std::regex_match(entry.path().filename().string(), pattern)) {
					std::string pic_name = entry.path().filename().string();
					pic_count++;
					auto it = std::find_if(client->pic_textures.begin(), client->pic_textures.end(), [&pic_name](const std::pair<std::string, Texture>& element) {
						return element.first == pic_name;
						});
					if (it != client->pic_textures.end()) {}
						//std::cout<<"FOUND!\n";
					else {
						Texture history_pic_texture(pd3ddevice);
						std::vector<uint8_t> image_data_history = readFileToVector(rpath + "/" + pic_name);
						history_pic_texture.LoadTextureFromMemory(image_data_history.data(), image_data_history.size());
						client->pic_textures.push_back(std::make_pair(pic_name, history_pic_texture));
					}

				}
				else {
					//std::cout << entry.path().filename() << " �����ϸ�ʽ" << std::endl;
				}
			}
			
		}
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "Filesystem error: " << e.what() << std::endl;
	}
	if (pic_count == 0) {
		ImGui::Text(u8"û���κ���ʷ��¼\n�������ŵ����¼��ʷ�İ�ť��������");
		ImGui::End();
		return;
	}
	ImGui::Text(u8"��%d����ʷ��¼",pic_count);
	ImGui::SameLine();
	if (ImGui::Button(u8"��Ϊ��Ƶ����")) {
		render_video = true;
		full_screen_pic_size = ImVec2(client->aspect_ratio * 1000, 1000);
	}
	
	ImVec2 pic_size(client->aspect_ratio * 100, 100);
	float windows_size_x = (int)ImGui::GetWindowSize().x;
	int redered_pic_count = 0;
	int column = 0;
	int row = 0;
	int max_x = int(ImGui::GetWindowSize().x) / 300;
	for (const auto& pair : client->pic_textures) {
		const std::string& name = pair.first;
		ImGui::SetCursorPos(ImVec2(column * 300 + 10, 100 + row * 150));
		column++;
		if (column >= max_x) {
			column = 0;
			row++;
		}
		ImGui::BeginChild(name.c_str(), ImVec2(300, 150), true);
		Texture texture = pair.second;
		ImGui::Image(texture.GetTexture(), pic_size); // ����ͼƬ
		if (ImGui::IsItemClicked())
		{
			std::cout << "Cliecked" << std::endl;
			Clicked_Full_screen_image = texture;
			show_full_screen_name = id + "/" + name;
			full_screen_pic_size = ImVec2(client->aspect_ratio * 1000, 1000);
			show_full_screen = true;
		}
		
		ImGui::Text(name.c_str());
		redered_pic_count++;
		ImGui::EndChild();
	}
	ImGui::End();
	if (show_full_screen)
	{
		ImGui::SetNextWindowSize(ImVec2(full_screen_pic_size.x,full_screen_pic_size.y+50), ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(history_windows_location, ImGuiCond_Appearing);
		ImGui::StyleColorsDark();
		ImGui::Begin((show_full_screen_name + u8"��ʷ��¼��ͼչʾ").c_str(), &show_full_screen);
		ImGui::Image(Clicked_Full_screen_image.GetTexture(), full_screen_pic_size);
		history_windows_location = ImGui::GetWindowPos();
		ImGui::End();
		ImGui::StyleColorsLight();
	}
	if (render_video) {
		static int index_of_texture = 0;
		static unsigned int time_count = 0;
		ImGui::SetNextWindowSize(full_screen_pic_size, ImGuiCond_Appearing);
		ImGui::SetNextWindowPos(history_windows_location, ImGuiCond_Appearing);
		ImGui::StyleColorsDark();
		ImGui::Begin(u8"��������(��Ƶ)ÿ��һ֡", &render_video);
		const auto& pair = client->pic_textures[index_of_texture];
		if (GetTickCount64() - time_count > 1000) {
			index_of_texture = (index_of_texture + 1) % client->pic_textures.size();
			time_count = GetTickCount64();
		}
		Texture texture = pair.second;
		ImGui::Image(texture.GetTexture(), full_screen_pic_size);
		history_windows_location = ImGui::GetWindowPos();
		ImGui::End();
		ImGui::StyleColorsLight();
	}
	if (!*show_history) {
		for (auto& pair : client->pic_textures) {
			pair.second.Release_Texture();
		}
		show_full_screen = false;
		render_video = false;
		client->pic_textures.clear();
		client->pic_textures.shrink_to_fit(); // �����ͷ�δʹ�õ��ڴ�
	}
	return;
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
				ImGui::SliderInt(u8"�Զ�����ͼƬ(��ʷ)ʱ����(��)", &save_interval, 10, 120);
				client_number = 0;
				bool did_save = false;
				int row = 0;
				int column = 0;
				for (const auto& v : all_connected_clients) {//�������пͻ��ˣ��ֱ����
					ImGui::SetCursorPos(ImVec2(column * 300 + 10, 100 + row * 280));
					column++;
					if (column >= max_x) {
						column = 0;
						row++;
					}
					ClientHandler* now_draw_client = (ClientHandler*)v;
					ImGui::BeginChild(std::to_string(client_number).c_str(), ImVec2(300, 280), true);

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
					else {
						 std::string offline_time_str = std::to_string((GetTickCount64() - now_draw_client->offline_time) / 1000) + u8"��";
						ImGui::Text((now_draw_client->client_info + u8" ����" + offline_time_str).c_str());
					}
					ImGui::Image(now_draw_client->thumb_texture.GetTexture(), ImVec2(now_draw_client->aspect_ratio * 100, 100)); // ����ͼƬ
					std::ostringstream oss, buttonlable, tickbox,delete_button,history;
					oss << u8"���֡�� " << client_number;
					buttonlable << u8"ȫ���ͻ�" << client_number;
					tickbox << u8"�Զ���ͼ(������ʷ)" << client_number;
					delete_button << u8"ɾ��" << client_number;
					history << u8"�鿴��ʷ" << client_number;
					ImGui::SliderInt(oss.str().c_str(), &((ClientHandler*)v)->frame_rate, 1, 100);
					if (ImGui::Button(buttonlable.str().c_str())) {
						FullWindow = true;
						switched = true;
						MainMonitoring = (ClientHandler*)v;
					}
					ImGui::SameLine();
					//����Ƿ��û������CheckBox
					bool tmp_tick = now_draw_client->save_image;
					ImGui::Checkbox(tickbox.str().c_str(), &now_draw_client->save_image);
					if (!tmp_tick == now_draw_client->save_image) save_time = 0;//���������ۼ�ʱ������ ��������ͼƬ���¿�ʼ��ʱ
					now_draw_client->main_monitoring = now_draw_client->save_image;//���ȷ��Ҫ����������Ϊ��֪ͨ�ͻ���Ҫ��ȡ�߷ֱ��ʵ�ͼƬ����Ϊ��һ�μ���ͼƬ��ʱ��Ҫ������
					if (now_draw_client->save_image && GetTickCount64() - save_time > save_interval * 1000) {
						now_draw_client->able_to_save = true;//ֻ�е�ѡ�񱣴���������Ҽ�ʱ�Ѿ������û�����ֵ�����ʶλ��Ϊ��
						did_save = true;//������ѭ�������ü�ʱ
					}
					if (ImGui::Button(history.str().c_str())) {
						now_draw_client->show_history = true;
						for (const auto& other : all_connected_clients) {
							if ((ClientHandler*)other != now_draw_client && ((ClientHandler*)other)->show_history == true) {
								((ClientHandler*)other)->show_history = false;
								for (auto& pair : ((ClientHandler*)other)->pic_textures) {
									pair.second.Release_Texture();
								}
								((ClientHandler*)other)->pic_textures.clear();
								((ClientHandler*)other)->pic_textures.shrink_to_fit();
							}
						}
					}
					if (now_draw_client->show_history) {
						Render_History(now_draw_client,(now_draw_client->id).c_str(),&now_draw_client->show_history, pd3ddevice);
					}
					if (now_draw_client->offline_too_long_able_to_delete) {
						ImGui::SameLine();
						if (ImGui::Button(delete_button.str().c_str())) {
							now_draw_client->do_delete_this_client = true;
						}
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
