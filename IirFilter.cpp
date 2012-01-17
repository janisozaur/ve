#include "IirFilter.h"

#include <cstring>

IirFilter::IirFilter(double *pdFirCoeffs, int uiNumFirCoeffs,
                double *pdIirCoeffs, int uiNumIirCoeffs)
{
    m_uiNumFirCoeffs = uiNumFirCoeffs;
    m_uiNumIirCoeffs = uiNumIirCoeffs;
    m_pdFirCoeffs = new double[uiNumFirCoeffs];
    m_pdInputBuf = new double[uiNumFirCoeffs];
    m_pdIirCoeffs = new double[uiNumIirCoeffs];
    m_pdOutputBuf = new double[uiNumIirCoeffs];

    memcpy(m_pdFirCoeffs, pdFirCoeffs, sizeof(double) * uiNumFirCoeffs);
    memset(m_pdInputBuf, 0, sizeof(double) * uiNumFirCoeffs);
    memcpy(m_pdIirCoeffs, pdIirCoeffs, sizeof(double) * uiNumIirCoeffs);
    memset(m_pdOutputBuf, 0, sizeof(double) * uiNumIirCoeffs);
}

IirFilter::~IirFilter() {
    delete[] m_pdFirCoeffs;
    delete[] m_pdInputBuf;
    delete[] m_pdIirCoeffs;
    delete[] m_pdOutputBuf;
}

double IirFilter::step(double input) {
    double sum = 0;

    for (size_t i = m_uiNumFirCoeffs - 1; i > 0; i--) {
        m_pdInputBuf[i] = m_pdInputBuf[i-1];
        sum += m_pdFirCoeffs[i] * m_pdInputBuf[i];
    }
    m_pdInputBuf[0] = input;
    sum += m_pdFirCoeffs[0] * m_pdInputBuf[0];

    for (size_t i = m_uiNumIirCoeffs - 1; i > 0; i--) {
        m_pdOutputBuf[i] = m_pdOutputBuf[i-1];
        sum -= m_pdIirCoeffs[i] * m_pdOutputBuf[i];
    }
    m_pdOutputBuf[0] = sum;

    return sum;
}
