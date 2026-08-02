import { MimeTypeImage } from '$lib/enums';
import { HEIC_JPEG_QUALITY } from '$lib/constants/image-size';
// heic requires a relatively large decoder, in order to reduce primary bundle size
// we lazily load this decoder from a CDN when needed, and cache it for future conversions
const HEIC_TO_CDN_URL = 'https://cdn.jsdelivr.net/npm/heic-to@1.5.2/dist/heic-to.js';
let modulePromise = null;
/**
 * Lazily load the heic-to decoder from the CDN and cache it
 * @returns Promise resolving to the heic-to module
 */
function getHeicTo() {
    if (!modulePromise) {
        modulePromise = import(/* @vite-ignore */ HEIC_TO_CDN_URL);
    }
    return modulePromise;
}
/**
 * Convert a HEIC/HEIF file to a compressed JPEG data URL
 * @param file - The HEIC/HEIF file to convert
 * @returns Promise resolving to JPEG data URL
 */
export async function heicFileToJpegDataURL(file) {
    const { heicTo } = await getHeicTo();
    const jpegBlob = await heicTo({
        blob: file,
        type: MimeTypeImage.JPEG,
        quality: HEIC_JPEG_QUALITY
    });
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = () => resolve(reader.result);
        reader.onerror = () => reject(reader.error);
        reader.readAsDataURL(jpegBlob);
    });
}
/**
 * Check if a MIME type represents a HEIC/HEIF image
 * @param mimeType - The MIME type to check
 * @returns True if the MIME type is image/heic or image/heif
 */
export function isHeicMimeType(mimeType) {
    const normalized = mimeType.trim().toLowerCase();
    return normalized === MimeTypeImage.HEIC || normalized === MimeTypeImage.HEIF;
}
