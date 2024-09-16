// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GenerateDevicePool.h"

#include <QDateTime>
#include <QLoggingCategory>

#include "DeviceGenerator.h"
#include "DeviceFactory.h"
#include "DeviceManager.h"

GenerateTask::GenerateTask(DeviceType deviceType)
    : m_Type(deviceType)
{

}

GenerateTask::~GenerateTask()
{

}

void GenerateTask::run()
{
    DeviceGenerator *generator = DeviceFactory::getDeviceGenerator();

    if (!generator)
        return;

    switch (m_Type) {
    case DT_Computer:
        generator->generatorComputerDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Cpu:
        generator->generatorCpuDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Bios:
        generator->generatorBiosDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Memory:
        generator->generatorMemoryDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Storage:
        generator->generatorDiskDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Gpu:
        generator->generatorGpuDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Monitor:
        generator->generatorMonitorDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Network:
        generator->generatorNetworkDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Audio:
        generator->generatorAudioDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Bluetoorh:
        generator->generatorBluetoothDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Keyboard:
        generator->generatorKeyboardDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Mouse:
        generator->generatorMouseDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Print:
        generator->generatorPrinterDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Image:
        generator->generatorCameraDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Cdrom:
        generator->generatorCdromDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Others:
        generator->generatorOthersDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    case DT_Power:
        generator->generatorPowerDevice();
        generator->generatorInfoFromToml(m_Type);
        break;
    default:
        break;
    }

    emit finished(generator->getBusIDFromHwinfo());
    delete generator;
    generator = nullptr;
}


GenerateDevicePool::GenerateDevicePool()
    : QThreadPool()
{
    initType();
}

void GenerateDevicePool::generateDevice()
{
    m_FinishedGenerator = 0;

    QList<DeviceType>::iterator it = m_TypeList.begin();
    for (; it != m_TypeList.end(); ++it) {
        GenerateTask *task = new GenerateTask((*it));
        connect(task, &GenerateTask::finished, this, &GenerateDevicePool::slotFinished);
        QThreadPool::globalInstance()->start(task);
//        task->setAutoDelete(true);
    }

    // 当所有设备执行完毕之后，开始执行生成其它设备的任务
    // 这里是为了确保其它设备在最后一个生成
    qint64 beginMSecond = QDateTime::currentMSecsSinceEpoch();
    while (true) {
        qint64 curMSecond = QDateTime::currentMSecsSinceEpoch();
        if (m_FinishedGenerator == m_TypeList.size()  || curMSecond - beginMSecond > 4000) {
            DeviceGenerator *generator = DeviceFactory::getDeviceGenerator();
            generator->generatorOthersDevice();
            generator->generatorInfoFromToml(DT_Others);

            // 指针使用结束释放
            delete generator;
            generator = nullptr;

            break;
        }
    }
}

void GenerateDevicePool::initType()
{
    m_TypeList.push_back(DT_Bluetoorh);
    m_TypeList.push_back(DT_Keyboard);
    m_TypeList.push_back(DT_Mouse);
    m_TypeList.push_back(DT_Image);
    m_TypeList.push_back(DT_Storage);
    m_TypeList.push_back(DT_Computer);
    m_TypeList.push_back(DT_Cpu);
    m_TypeList.push_back(DT_Bios);
    m_TypeList.push_back(DT_Memory);
    m_TypeList.push_back(DT_Gpu);
    m_TypeList.push_back(DT_Monitor);
    m_TypeList.push_back(DT_Network);
    m_TypeList.push_back(DT_Audio);
    m_TypeList.push_back(DT_Print);
    m_TypeList.push_back(DT_Cdrom);
    m_TypeList.push_back(DT_Power);
//    m_TypeList.push_back(DT_Others);
}

void GenerateDevicePool::slotFinished(const QStringList &lst)
{
    DeviceManager::instance()->addBusId(lst);
    m_FinishedGenerator++;
}
