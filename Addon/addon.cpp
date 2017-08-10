#include <utility>
#include <algorithm>
#include "SnailAddonHost.h"

/*
���Խ���:
�����ߴ���3������5,10,30�վ���,�ڶ��տ��������������������,�����Խ����������Բο�
*/

SnailAddonHost* addonHost = nullptr;
StockIndicator* indicator = nullptr;
StockIndicator* indicator5 = nullptr;
StockIndicator* indicator10 = nullptr;

extern "C" SNAIL_API_EXPORT SnailAddon* get_snail_interface(SnailAddonHost* host,int flag);

inline bool isHarden(const Stock& stock){return stock.increase > 9.6f;}
inline bool isEqual(float a,float b){return fabsf(a-b) < 0.02f;}

QString maker_ = "��ţ(snail007.com)";
QString description_ = "һ�������߲���";

#define CHECK_AND_ADD(j)\
	std::tuple<QString,float> buy(QString("��һ������"),0.0f);\
	if(j < list.size())\
	    buy = std::make_tuple<QString,float>(list[j].date,list[j].start);\
	std::tuple<QString,float> sale(QString("δ֪"),0.0f);\
	if(j+1<list.size())\
        sale = std::make_tuple<QString,float>(list[j+1].date,list[j+1].current);\
	addonHost->append(code,name,buy,sale,description_); 

class SnailAddonImpl : public SnailAddon
{
	void destroy(){delete this;}
	const QString maker()const{return maker_;}
	const QString version()const{return "1.0.0.3";}
	const QString description()const{return description_;}
	const QString tips()const{return "��ţ��׼����";}
    void onRecevied(const QString& code,const QVector<Stock>& list)
	{
		const int span = 31;
	    if(list.size() <= span)
			return;

		//��ȡ��Ʊ�ع�Ʊ�б�
		auto set = addonHost->getStockList();
		QString name;
		if(set.contains(code))
		    name = set[code].name;

		indicator->clear();
		QVector<std::tuple<bool,float,float,float,float> > cv,cv5,cv10;
		//��������3����ָ��
		foreach(Stock stock,list)
		{
			cv.append(indicator->update(stock));
			cv5.append(indicator->update(stock));
			cv10.append(indicator->update(stock));
		}

		for(int i=span;i<list.size()-1;i++)
		{
			//����������ƫ���
			float delta = std::max(std::get<1>(cv[i]),std::max(std::get<1>(cv5[i]),std::get<1>(cv10[i])));
			delta -= std::min(std::get<1>(cv[i]),std::min(std::get<1>(cv5[i]),std::get<1>(cv10[i])));
			delta /= std::min(std::get<1>(cv[i]),std::min(std::get<1>(cv5[i]),std::get<1>(cv10[i])));
			delta *= 100.0f;

			//�Ƿ�����7�������ߴ������ߣ�����ǰһ���ڿ�����ߵ�һ��
			if(list[i].increase >= 7.0f && 
			   std::get<1>(cv[i]) < list[i].current && std::get<1>(cv[i]) > list[i].start &&
			   std::get<1>(cv5[i]) > list[i].start && std::get<1>(cv5[i]) < list[i].current &&
			   std::get<1>(cv10[i]) > list[i].start && std::get<1>(cv10[i]) < list[i].current &&
			   std::get<1>(cv[i]) > std::get<1>(cv5[i]) && std::get<1>(cv[i]) > std::get<1>(cv10[i]) && 
			   std::get<1>(cv[i-1]) > std::max(list[i-1].start,list[i-1].current) &&
			   std::get<1>(cv5[i-1]) > std::max(list[i-1].start,list[i-1].current) &&
               std::get<1>(cv10[i-1]) > std::max(list[i-1].start,list[i-1].current) &&
			   delta < 3.6f)
			{
				CHECK_AND_ADD(i+1)
			}
		}
	}
private:
};

SnailAddon* get_snail_interface(SnailAddonHost* host,int flag)
{
	if(flag != SNAIL_ADDON_VERSION_INT || !host)
		return nullptr;

	addonHost = host;
	indicator = addonHost->createIndicator(SnailAddonHost::TAG_Indicator_Ma,30,0,0);
	indicator5 = addonHost->createIndicator(SnailAddonHost::TAG_Indicator_Ma,5,0,0);
	indicator10 = addonHost->createIndicator(SnailAddonHost::TAG_Indicator_Ma,10,0,0);
    return new SnailAddonImpl();
}
