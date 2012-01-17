#ifndef IIRFILTER_H
#define IIRFILTER_H

class IirFilter {
public:
    IirFilter(double *pdFirCoeffs, int uiNumFirCoeffs,
              double *pdIirCoeffs, int uiNumIirCoeffs);
    ~IirFilter();
    double step(double input);

private:
    double *m_pdFirCoeffs;
    double *m_pdIirCoeffs;
    double *m_pdInputBuf;
    double *m_pdOutputBuf;
    int m_uiNumFirCoeffs;
    int m_uiNumIirCoeffs;
};

#endif // IIRFILTER_H
